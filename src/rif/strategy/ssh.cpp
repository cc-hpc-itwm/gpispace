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

namespace
{
  //! \todo former ssh options: -T -n

  struct ssh2
  {
    ssh2()
    {
      if (int rc = libssh2_init (0))
      {
        throw std::runtime_error
          ("libssh2 initialization failed: " + std::to_string (rc));
      }
    }

    ~ssh2()
    {
      libssh2_exit();
    }

    struct channel;

    struct session
    {
      session ( int socket
              , std::string const& username
              , boost::filesystem::path const& public_key
              , boost::filesystem::path const& private_key
              )
        : _socket (socket)
        , _ (libssh2_session_init())
      {
        if (!_)
        {
          throw std::runtime_error ("libssh2 session initialization failed");
        }

        //! \todo do blocking (check if eagain can still happen)
        libssh2_session_set_blocking (_, 0);

        handshake();
        authenticate (username, public_key, private_key);
      }
      ~session()
      {
        libssh2_session_disconnect (_, "");
        libssh2_session_free (_);
      }

    private:
      //! \todo remove when blocking
      void waitsocket (std::chrono::seconds tout)
      {
        fd_set fd;
        FD_ZERO (&fd);

        FD_SET (_socket, &fd);

        int const dir (libssh2_session_block_directions (_));

        fd_set* writefd (nullptr);
        fd_set* readfd (nullptr);

        if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        {
          readfd = &fd;
        }

        if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        {
          writefd = &fd;
        }

        struct timeval timeout;
        timeout.tv_sec = tout.count();
        timeout.tv_usec = 0;

        fhg::util::syscall::select (_socket + 1, readfd, writefd, NULL, &timeout);
      }

      void handshake()
      {
        int rc;
        while ( (rc = libssh2_session_handshake (_, _socket))
               == LIBSSH2_ERROR_EAGAIN
              )
        {
          //! \note retry
          //! \todo retry counter?
        }

        if (rc)
        {
          throw std::runtime_error
            ("libssh2: could not handshake SSH session: " + std::to_string (rc));
        }
      }

      void authenticate ( std::string const& username
                        , boost::filesystem::path const& public_key
                        , boost::filesystem::path const& private_key
                        )
      {
        int rc;
        while ( (rc = libssh2_userauth_publickey_fromfile
                    ( _
                    , username.c_str()
                    , public_key.string().c_str()
                    , private_key.string().c_str()
                    , nullptr
                    )
                )
               == LIBSSH2_ERROR_EAGAIN
              )
        {
          //! \note retry
          //! \todo retry counter?
        }

        if (rc)
        {
          throw std::runtime_error
            ("libssh2: could not authenticate SSH session: " + std::to_string (rc));
        }
      }

      int _socket;
      LIBSSH2_SESSION* _;

      friend struct channel;
    };

    struct channel
    {
      channel (ssh2::session& session)
        : _session (session)
      {
        int rc;
        /* Exec non-blocking on the remove host */
        while( (_ = libssh2_channel_open_session (_session._)) == nullptr &&
               (rc = libssh2_session_last_error (_session._, nullptr, nullptr,0))
               == LIBSSH2_ERROR_EAGAIN
        )
        {
          _session.waitsocket (std::chrono::seconds (1));
        }
        if (!_)
        {
          throw std::runtime_error
            ("libssh2: could not open channel: " + std::to_string (rc));
        }
      }

      std::string read_from_channel (int stream_id)
      {
        std::vector<char> buffer (0x1000);
        std::size_t bytes_read_total (0);

        while (true)
        {
          ssize_t const bytes_read_or_rc
            ( libssh2_channel_read_ex ( _
                                      , stream_id
                                      , buffer.data() + bytes_read_total
                                      , buffer.size() - bytes_read_total
                                      )
            );

          if (bytes_read_or_rc > 0)
          {
            if (buffer.size() - bytes_read_total == bytes_read_or_rc)
            {
              buffer.resize (buffer.size() * 2);
            }

            bytes_read_total += bytes_read_or_rc;
          }
          //\todo check condition, is == 0 enough?
          else if ( bytes_read_or_rc == LIBSSH2_ERROR_CHANNEL_CLOSED
                  || bytes_read_or_rc == 0
                  )
          {
            break;
          }
          else if (bytes_read_or_rc == LIBSSH2_ERROR_EAGAIN)
          {
            _session.waitsocket (std::chrono::seconds (1));
          }
          else
          {
            throw std::runtime_error
              ( "error reading from channel: "
              + std::to_string (bytes_read_or_rc)
              );
          }
        }

        return {buffer.begin(), buffer.begin() + bytes_read_total};
      }

      std::tuple<int, std::string, std::string> execute
        (std::string const& command)
      {
        int rc;
        while ( (rc = libssh2_channel_exec (_, command.c_str()))
              == LIBSSH2_ERROR_EAGAIN
              )
        {
          _session.waitsocket (std::chrono::seconds (1));
        }

        if (rc)
        {
          throw std::runtime_error
            ("libssh2: could not execute '" + command + "': " + std::to_string (rc));
        }

        std::string const stdout (read_from_channel (0));
        std::string const stderr (read_from_channel (SSH_EXTENDED_DATA_STDERR));

        int const exitcode (libssh2_channel_get_exit_status (_));
        char* exitsignal (nullptr);
        if ( (rc = libssh2_channel_get_exit_signal
               (_, &exitsignal, nullptr, nullptr, nullptr, nullptr, nullptr)
             )
           )
        {
          throw std::runtime_error
            ("Could not get exit_signal: " + std::to_string (rc));
        }

        if (exitsignal)
        {
          throw std::runtime_error ("signaled: " + std::string (exitsignal));
        }

        return std::make_tuple (exitcode, stdout, stderr);
      }

      ~channel()
      {
        int rc;
        while ( (rc = libssh2_channel_close (_))
              == LIBSSH2_ERROR_EAGAIN
              )
        {
          _session.waitsocket (std::chrono::seconds (1));
        }

        if (rc)
        {
          throw std::runtime_error
            ("libssh2: could not close channel: " + std::to_string (rc));
        }

        libssh2_channel_free (_);
      }

    private:
      session& _session;
      LIBSSH2_CHANNEL* _;
    };
  };

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

                ssh2 _;

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
                        , [hostname, command, &username, &public_key, &private_key]
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

                            ssh2::session session (sock, username, public_key, private_key);
                            ssh2::channel channel (session);
                            auto output (channel.execute (command));

                            if (std::get<0> (output))
                            {
                              throw std::runtime_error (command + " on " + hostname + " failed: " + std::to_string (std::get<0> (output)) + " out: '" + std::get<1> (output) + "' err: '" + std::get<2> (output) + "'");
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
