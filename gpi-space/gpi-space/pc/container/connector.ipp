/* -*- mode: c++ -*- */
#include "connector.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <boost/bind.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template <typename M>
      connector_t<M>::~connector_t ()
      {
        try
        {
          stop ();
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "error within ~connector_t: " << ex.what());
        }
      }

      template <typename M>
      void connector_t<M>::start ()
      {
        lock_type lock (m_mutex);

        if (m_socket >= 0)
          return;

        m_stopping = false;
        int err = safe_unlink (m_path);
        if (err < 0)
        {
          LOG(ERROR, "could not unlink path " << m_path << ": " <<  strerror(-err));
          throw std::runtime_error ("could not unlink socket path");
        }

        int fd (open_socket (m_path));
        if (fd < 0)
        {
          LOG(ERROR, "could not open socket: " << strerror(-fd));
          throw std::runtime_error ("could not open socket");
        }
        else
        {
          m_socket = fd;
          DLOG(DEBUG, "listening on " << m_path << " (fd " << fd << ")");
          start_thread ();
        }
      }

      template <typename M>
      void connector_t<M>::stop ()
      {
        lock_type lock (m_mutex);
        if (m_socket >= 0)
        {
          m_stopping = true;

          safe_unlink (m_path);
          close_socket (m_socket);
          stop_thread ();
          m_socket = -1;
        }
      }

      template <typename M>
      void connector_t<M>::start_thread ()
      {
        assert (m_socket >= 0);

        m_listener = thread_t
          (new boost::thread(boost::bind( &self::listener_thread_main
                                        , this
                                        , m_socket
                                        )
                            )
          );
      }

      template <typename M>
      void connector_t<M>::stop_thread ()
      {
        assert (m_listener);
        if (boost::this_thread::get_id() != m_listener->get_id())
        {
          m_listener->join ();
          m_listener.reset ();
        }
      }

      template <typename M>
      int connector_t<M>::close_socket (const int fd)
      {
        shutdown (fd, SHUT_RDWR);
        return close (fd);
      }

      template <typename M>
      int connector_t<M>::open_socket (std::string const & path)
      {
        int sfd, err;
        struct sockaddr_un my_addr;
        const std::size_t backlog_size (16);

        sfd = socket (AF_UNIX, SOCK_STREAM, 0);
        if (sfd == -1)
        {
          err = errno;
          LOG(ERROR, "could not create unix socket: " << strerror(err));
          return -err;
        }
        setsockopt (sfd, SOL_SOCKET, SO_PASSCRED, (void*)1, 1);

        memset (&my_addr, 0, sizeof(my_addr));
        my_addr.sun_family = AF_UNIX;
        strncpy ( my_addr.sun_path
                , path.c_str()
                , sizeof(my_addr.sun_path) - 1
                );

        if (bind( sfd
                , (struct sockaddr *)&my_addr
                , sizeof(struct sockaddr_un)
                ) == -1
           )
        {
          err = errno;
          LOG(ERROR, "could not bind to socket at path " << path << ": " << strerror(err));
          close (sfd);
          return -err;
        }
        chmod (path.c_str(), 0700);

        if (listen(sfd, backlog_size) == -1)
        {
          err = errno;
          LOG(ERROR, "could not listen on socket: " << strerror(err));
          close (sfd);
          unlink (path.c_str());
          return -err;
        }

        return sfd;
      }

      template <typename M>
      int connector_t<M>::safe_unlink(std::string const & path)
      {
        struct stat st;
        int err (0);

        if (stat (path.c_str(), &st) == 0)
        {
          if (S_ISSOCK(st.st_mode))
          {
            unlink (path.c_str());
            err = 0;
          }
          else
          {
            err = EINVAL;
          }
        }

        return -err;
      }

      template <typename M>
      void connector_t<M>::listener_thread_main(const int fd)
      {
        int cfd, err;
        struct sockaddr_un peer_addr;
        socklen_t peer_addr_size;

        for (;;)
        {
          peer_addr_size = sizeof(struct sockaddr_un);
          cfd = accept( fd
                      , (struct sockaddr*)&peer_addr
                      , &peer_addr_size
                      );
          if (cfd == -1)
          {
            err = errno;
            if (! m_stopping)
            {
              LOG(ERROR, "could not accept: " << strerror(err));
              m_mgr.handle_connector_error (err);
            }
            break;
          }

          try
          {
            m_mgr.handle_new_connection (cfd);
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "could not handle new connection: " << ex.what());
            close_socket (cfd);
          }
        }
      }
    }
  }
}
