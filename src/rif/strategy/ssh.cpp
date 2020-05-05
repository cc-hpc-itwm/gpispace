// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/ssh.hpp>

#include <util-generic/blocked.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>

#include <rif/strategy/ssh/session.hpp>

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

            ~socket()
            {
              util::syscall::close (_fd);
            }

            int _fd;
          };

          namespace option
          {
            namespace validator = fhg::util::boost::program_options;

            constexpr char const* const block_size {"sshs-at-once"};
            constexpr char const* const block_size_description
              {"how many SSH connections to establish at once"};
            using block_size_type = std::size_t;
            using block_size_validator
              = validator::positive_integral<std::size_t>;
            constexpr block_size_type const block_size_default = 64;

            constexpr char const* const ssh_port {"ssh-port"};
            constexpr char const* const ssh_port_description
              {"port where sshd listens on remote hosts"};
            using ssh_port_type = unsigned short;
            using ssh_port_validator
              = validator::positive_integral<unsigned short>;
            constexpr ssh_port_type const ssh_port_default = 22;

            constexpr char const* const username {"ssh-username"};
            constexpr char const* const username_description
              {"username to use for remote host"};
            using username_type = std::string;
            using username_validator = validator::nonempty_string;
            username_type const username_default()
            {
              char* const user (util::syscall::getenv ("USER"));
              return user ? user : "";
            }

            constexpr char const* const public_key {"ssh-public-key"};
            constexpr char const* const public_key_description
              {"public key file used for authentication"};
            using public_key_type = boost::filesystem::path;
            //! \todo validate?
            using public_key_validator = public_key_type;
            public_key_type public_key_default()
            {
              char* const home (util::syscall::getenv ("HOME"));
              return home
                ? boost::filesystem::path (home) / ".ssh" / "id_rsa.pub"
                : boost::filesystem::path();
            }

            constexpr char const* const private_key {"ssh-private-key"};
            constexpr char const* const private_key_description
              {"private key file used for authentication"};
            using private_key_type = boost::filesystem::path;
            //! \todo validate?
            using private_key_validator = private_key_type;
            private_key_type private_key_default()
            {
              char* const home (util::syscall::getenv ("HOME"));
              return home
                ? boost::filesystem::path (home) / ".ssh" / "id_rsa"
                : boost::filesystem::path();
            }
          }

          //! \todo use fhg::util::boost::program_options::generic
#define EXTRACT_PARAMETERS(parameters_)                                 \
          boost::program_options::options_description options;          \
          options.add_options()                                         \
            ( option::block_size                                        \
            , boost::program_options::value<option::block_size_validator>() \
              ->default_value (option::block_size_default)              \
            , option::block_size_description                            \
            )                                                           \
            ( option::ssh_port                                          \
            , boost::program_options::value<option::ssh_port_validator>() \
              ->default_value (option::ssh_port_default)->required()    \
            , option::ssh_port_description                              \
            )                                                           \
            ( option::username                                          \
            , boost::program_options::value<option::username_validator>() \
              ->default_value (option::username_default())->required()  \
            , option::username_description                              \
            )                                                           \
            ( option::public_key                                        \
            , boost::program_options::value<option::public_key_validator>()\
              ->default_value (option::public_key_default())->required() \
            , option::public_key_description                            \
            )                                                           \
            ( option::private_key                                       \
            , boost::program_options::value<option::private_key_validator>() \
              ->default_value (option::private_key_default())->required() \
            , option::private_key_description                           \
            )                                                           \
            ;                                                           \
                                                                        \
          boost::program_options::variables_map vm;                     \
          boost::program_options::store                                 \
            ( boost::program_options::command_line_parser (parameters)  \
              .options (options).run()                                  \
            , vm                                                        \
            );                                                          \
                                                                        \
          boost::program_options::notify (vm);                          \
                                                                        \
          boost::optional<option::block_size_type> const block_size     \
            ( vm.count (option::block_size)                             \
            ? boost::optional<option::block_size_type>                  \
                (vm.at (option::block_size).as<option::block_size_validator>()) \
            : boost::none                                               \
            );                                                          \
          option::username_type const username                          \
            (vm.at (option::username).as<option::username_validator>()); \
          option::ssh_port_type const ssh_port                          \
            (vm.at (option::ssh_port).as<option::ssh_port_validator>()); \
          option::public_key_type const public_key                      \
            (vm.at (option::public_key).as<option::public_key_validator>()); \
          option::private_key_type const private_key                    \
            (vm.at (option::private_key).as<option::private_key_validator>())

        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
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
                         (( boost::format
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
                  (( boost::format ("/bin/kill -TERM %1%")
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
