// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/ssh.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
//! \todo remove
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>

#include <future>
#include <mutex>
#include <stdexcept>

#include <libssh2.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include <libssh2_wrapper/session.hpp>
#include <util-generic/unreachable.hpp>

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
          template<typename Fun, typename Container>
            void blocked (Container const& container, Fun&& fun)
          {
            //! \todo 112 -> parameter
            std::size_t const block_size (112);
            std::size_t position (0);

            while (position < container.size())
            {
              std::size_t const count
                (std::min (block_size, container.size() - position));

              fun ( Container ( std::next (container.begin(), position)
                              , std::next (container.begin(), position + count)
                              )
                  );

              position += count;
            }
          }

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

            ~socket()
            {
              util::syscall::close (_fd);
            }

            int _fd;
          };
        }

        void bootstrap ( std::vector<std::string> const& all_hostnames
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       )
        {
          blocked
            ( all_hostnames
            , [&] (std::vector<std::string> const& hostnames)
              {
                std::vector<std::future<void>> sshs;

                //! \todo parameter with default
                std::string username ("loerwald");
                unsigned short ssh_port (22);
                boost::filesystem::path public_key ("/u/l/loerwald/.ssh/fhg.pub");
                boost::filesystem::path private_key ("/u/l/loerwald/.ssh/fhg");

                libssh2::context ssh_context;

                for (std::string const& hostname : hostnames)
                {
                  std::string const command
                    ( ( boost::format
                          ("%1% %2% --register-host %3% --register-port %4%")
                      % binary
                      % (port ? "--port " + std::to_string (*port) : "")
                      % register_host
                      % register_port
                      ).str()
                    );

                  sshs.emplace_back
                    ( std::async
                        ( std::launch::async
                        , [ hostname, command, &username, &public_key
                          , &private_key, &ssh_context, &ssh_port
                          ]
                          {
                            fhg::util::nest_exceptions<std::runtime_error>
                              ( [&]
                                {
                                  socket const sock (hostname, ssh_port);
                                  libssh2::session session
                                    ( ssh_context
                                    , sock._fd
                                    , username
                                    , {public_key, private_key}
                                    );
                                  session.execute_and_require_success_and_no_output
                                    (command);
                                }
                              , hostname
                              );
                          }
                        )
                    );
                }

                fhg::util::wait_and_collect_exceptions (sshs);
              }
            );
        }

        void teardown ( std::vector<fhg::rif::entry_point> const& all_entry_points
                      , std::vector<fhg::rif::entry_point>& failed_entry_points
                      )
        {
          blocked
            ( all_entry_points
            , [&] (std::vector<fhg::rif::entry_point> const& entry_points)
              {
                //! \todo parameter with default
                std::string username ("loerwald");
                unsigned short ssh_port (22);
                boost::filesystem::path public_key ("/u/l/loerwald/.ssh/fhg.pub");
                boost::filesystem::path private_key ("/u/l/loerwald/.ssh/fhg");

                libssh2::context ssh_context;

                std::mutex failed_entry_points_guard;

                std::vector<std::future<void>> sshs;

                for (fhg::rif::entry_point const& entry_point : entry_points)
                {
                  std::string const command
                    ( ( boost::format ("/bin/kill -TERM %1%")
                      % entry_point.pid
                      ).str()
                    );

                  sshs.emplace_back
                    ( std::async
                        ( std::launch::async
                        , [ command, entry_point
                          , &failed_entry_points_guard, &failed_entry_points
                          , &username, &public_key, &private_key, &ssh_context
                          , &ssh_port
                          ]
                          {
                            fhg::util::nest_exceptions<std::runtime_error>
                              ( [&]
                                {
                                  try
                                  {
                                    socket const sock (entry_point.hostname, ssh_port);
                                    libssh2::session session
                                      ( ssh_context
                                      , sock._fd
                                      , username
                                      , {public_key, private_key}
                                      );
                                    session.execute_and_require_success_and_no_output
                                      (command);
                                  }
                                  catch (...)
                                  {
                                    std::unique_lock<std::mutex> const _
                                      (failed_entry_points_guard);
                                    failed_entry_points.emplace_back (entry_point);

                                    throw;
                                  }
                                }
                              , entry_point.hostname
                              );
                          }
                        )
                    );
                }

                fhg::util::wait_and_collect_exceptions (sshs);
              }
            );
        }
      }
    }
  }
}
