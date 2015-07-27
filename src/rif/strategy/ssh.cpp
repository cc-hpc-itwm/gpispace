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

namespace
{
  std::string resolve (std::string const& hostname)
  {
    struct addrinfo hints;

    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* servinfo {nullptr};
    struct free_addrinfo_on_scope_exit
    {
      free_addrinfo_on_scope_exit (struct addrinfo* addrinfo)
        : _ (addrinfo)
      {}
      ~free_addrinfo_on_scope_exit()
      {
        //! \todo move to syscall
        freeaddrinfo (_);
      }
      struct addrinfo* _;
    } const _ {servinfo};

    //! \todo move to syscall
    if (int rv = getaddrinfo (hostname.data(), nullptr, &hints , &servinfo))
    {
      //! \todo specific exception
      throw std::runtime_error
        (std::string ("resolve: getaddrinfo: ") + gai_strerror (rv));
    }

    for (struct addrinfo* p (servinfo); p != nullptr; p = p->ai_next)
    {
      struct sockaddr_in* h
        (reinterpret_cast<struct sockaddr_in *> (p->ai_addr));

      return inet_ntoa (h->sin_addr);
    }

    //! \todo specific exception
    throw std::runtime_error
      ("resolve: could not get ip for '" + hostname + "'");
  }
}

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace ssh
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
                std::string username ("rahn");
                boost::filesystem::path public_key ("/u/r/rahn/.ssh/id_rsa.pub");
                boost::filesystem::path private_key ("/u/r/rahn/.ssh/id_rsa");

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
                        , [hostname, command, &username, &public_key, &private_key, &ssh_context]
                          {
                            try
                            {
                              //! \todo connected_socket (scoped)
                              int sock = fhg::util::syscall::socket (AF_INET, SOCK_STREAM, 0);

                              {
                                struct sockaddr_in sin;
                                sin.sin_family = AF_INET;
                                sin.sin_port = htons (22);
                                sin.sin_addr.s_addr =
                                  inet_addr (resolve (hostname).data());
                                fhg::util::syscall::connect
                                  (sock, (struct sockaddr const*)&sin, sizeof sin);
                              }

                              libssh2::session session (ssh_context, sock, username, {public_key, private_key});
                              session.execute_and_require_success_and_no_output (command);
                            }
                            catch (...)
                            {
                              std::throw_with_nested (std::runtime_error (hostname));
                            }
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
                std::mutex failed_entry_points_guard;

                std::vector<std::future<void>> sshs;

                for (fhg::rif::entry_point const& entry_point : entry_points)
                {
                  std::string const command
                    ( ( boost::format ("ssh %1% %2% /bin/kill -TERM %3%")
                      % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
                      % entry_point.hostname
                      % entry_point.pid
                      ).str()
                    );

                  sshs.emplace_back
                    ( std::async
                        ( std::launch::async
                        , [ command, entry_point
                          , &failed_entry_points_guard, &failed_entry_points
                          ]
                          {
                            fhg::util::nest_exceptions<std::runtime_error>
                              ([&]()
                               {
                                 try
                                 {
                                   //! \todo use libssh2
                                   fhg::util::system_with_blocked_SIGCHLD (command);
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
