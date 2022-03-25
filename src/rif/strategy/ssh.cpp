// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <rif/strategy/ssh.hpp>

#include <util-generic/blocked.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/boost/program_options/validators/nonempty_file.hpp>
#include <util-generic/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <rif/strategy/ssh/session.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <future>
#include <stdexcept>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace ssh
      {
        namespace
        {
          struct socket
          {
            socket (std::string const& host, unsigned short port)
            try
              : _fd (-1)
            {
              ::addrinfo hints;
              memset (&hints, 0, sizeof hints);

              hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG | AI_V4MAPPED;
              hints.ai_family = AF_UNSPEC;
              hints.ai_socktype = SOCK_STREAM;

              util::syscall::addrinfo_ptr info
                ( util::syscall::getaddrinfo
                    (host.c_str(), std::to_string (port).c_str(), &hints)
                );

              std::vector<std::exception_ptr> exceptions;
              for ( ::addrinfo* ai (info.get())
                  ; ai && _fd == -1
                  ; ai = ai->ai_next
                  )
              {
                try
                {
                  _fd = util::syscall::socket ( ai->ai_family
                                              , ai->ai_socktype
                                              , ai->ai_protocol
                                              );
                  try
                  {
                    util::syscall::connect
                      (_fd, ai->ai_addr, ai->ai_addrlen);
                  }
                  catch (...)
                  {
                    util::syscall::close (_fd);
                    throw;
                  }

                  break;
                }
                catch (...)
                {
                  exceptions.emplace_back (std::current_exception());
                }
              }

              util::throw_collected_exceptions (exceptions);
            }
            catch (...)
            {
              std::throw_with_nested
                ( std::runtime_error
                    ( "resolve_and_connect (" + host + ", "
                    + std::to_string (port) + ")"
                    )
                );
            }

            socket() = delete;
            socket (socket const&) = delete;
            socket (socket&&) = delete;
            socket& operator= (socket const&) = delete;
            socket& operator= (socket&&) = delete;
            ~socket()
            {
              util::syscall::close (_fd);
            }

            int _fd;
          };

          namespace option
          {
            namespace po = fhg::util::boost::program_options;

            po::option<std::size_t, po::positive_integral<std::size_t>>
              const block_size { "sshs-at-once"
                               , "how many SSH connections to establish at once"
                               , 64
                               };

            po::option<unsigned short, po::positive_integral<unsigned short>>
              const ssh_port { "ssh-port"
                             , "port where sshd listens on remote host"
                             , 22
                             };

            namespace
            {
              po::option<std::string, po::nonempty_string> username()
              {
                auto const name ("ssh-username");
                auto const description ("username to use for remote host");

                ::boost::optional<std::string> const user (util::getenv ("USER"));
                if (user && !user->empty())
                {
                  return {name, description, *user};
                }
                else
                {
                  return {name, description};
                }
              }

              ::boost::optional<::boost::filesystem::path> maybe_default_key
                (std::string suffix)
              {
                auto const home (util::getenv ("HOME"));
                if (home)
                {
                  auto const key_path ( ::boost::filesystem::path (*home)
                                      / ".ssh"
                                      / ("id_rsa" + suffix)
                                      );
                  if (::boost::filesystem::exists (key_path))
                  {
                    return key_path;
                  }
                }
                return ::boost::none;
              }

              po::option<::boost::filesystem::path, po::existing_path> public_key()
              {
                auto const name ("ssh-public-key");
                auto const description ("public key file used for authentication");

                if (auto const maybe_default = maybe_default_key (".pub"))
                {
                  return {name, description, *maybe_default};
                }
                else
                {
                  return {name, description};
                }
              }

              po::option<::boost::filesystem::path, po::existing_path> private_key()
              {
                auto const name ("ssh-private-key");
                auto const description ("private key file used for authentication");

                if (auto const maybe_default = maybe_default_key (""))
                {
                  return {name, description, *maybe_default};
                }
                else
                {
                  return {name, description};
                }
              }
            }
          }

#define EXTRACT_PARAMETERS(parameters_)                                     \
          auto const vm ( option::po::options ("ssh")                       \
                        . require (option::block_size)                      \
                        . require (option::ssh_port)                        \
                        . require (option::username())                      \
                        . require (option::public_key())                    \
                        . require (option::private_key())                   \
                        . store_and_notify (parameters_)                    \
                        );                                                  \
                                                                            \
          auto const block_size (option::block_size.get_from (vm));         \
          auto const username (option::username().get_from (vm));           \
          auto const ssh_port (option::ssh_port.get_from (vm));             \
          auto const public_key (option::public_key().get_from (vm));       \
          auto const private_key (option::private_key().get_from (vm))

        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , ::boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , ::boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream& out
                    )
        {
          EXTRACT_PARAMETERS (parameters);

          libssh2::context ssh_context;

          return util::blocked_async<std::string>
            ( all_hostnames
            , block_size
            , [] (std::string const& hostname) { return hostname; }
            , [&] (std::string const& hostname)
              {
                socket const sock (hostname, ssh_port);
                libssh2::session session ( ssh_context
                                         , sock._fd
                                         , username
                                         , {public_key, private_key}
                                         );
                out << session.execute_and_require_success_and_no_stderr_output
                         (( ::boost::format
                            ( "%1% %2% --register-host %3% --register-port %4%"
                            " --register-key %5%"
                            )
                          % binary
                          % (port ? "--port " + std::to_string (*port) : "")
                          % register_host
                          % register_port
                          % hostname
                          ).str()
                         );
              }
            ).second;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, fhg::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          EXTRACT_PARAMETERS (parameters);

          libssh2::context ssh_context;

          return util::blocked_async<std::string>
            ( all_entry_points
            , block_size
            , [] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                return entry_point.first;
              }
            , [&] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                socket const sock (entry_point.second.hostname, ssh_port);
                libssh2::session session ( ssh_context
                                         , sock._fd
                                         , username
                                         , {public_key, private_key}
                                         );
                session.execute_and_require_success_and_no_output
                  (( ::boost::format ("/bin/kill -TERM %1%")
                   % entry_point.second.pid
                   ).str()
                  );
              }
            );
        }
      }
    }
  }
}
