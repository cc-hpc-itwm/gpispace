/* -*- mode: c++ -*- */
#include "connector.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fhglog/minimal.hpp>

#include <boost/bind.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template <typename M, typename P>
      void connector_t<M,P>::start ()
      {
        if (m_listener) return;

        m_listener = thread_t
          (new boost::thread(boost::bind( &self::listener_thread
                                        , this
                                        )
                            )
          );
      }

      template <typename M, typename P>
      void connector_t<M,P>::stop ()
      {
        if (! m_listener) return;
        m_listener->interrupt ();
        m_listener->join ();
        m_listener.reset ();
      }

      template <typename M, typename P>
      void connector_t<M,P>::listener_thread()
      {
        int sfd, cfd, err;
        struct sockaddr_un my_addr, peer_addr;
        socklen_t peer_addr_size;
        const std::size_t backlog_size (16);

        sfd = socket (AF_UNIX, SOCK_STREAM, 0);
        if (sfd == -1)
        {
          LOG(ERROR, "could not open unix socket: " << strerror(errno));
          return;
        }

        memset (&my_addr, 0, sizeof(my_addr));
        my_addr.sun_family = AF_UNIX;
        strncpy (my_addr.sun_path, m_path.c_str(), sizeof(my_addr.sun_path) - 1);

        // if (do_unlink_on_startup)
        unlink (m_path.c_str());
        if (bind( sfd
                , (struct sockaddr *)&my_addr
                , sizeof(struct sockaddr_un)
                ) == -1)
        {
          LOG(ERROR, "could not bind to socket at path " << m_path << ": " << strerror(errno));
          close (sfd);
          return;
        }

        if (listen(sfd, backlog_size) == -1)
        {
          LOG(ERROR, "could not listen on socket: " << strerror(errno));
          close (sfd);
          unlink (m_path.c_str());
          return;
        }

        for (;;)
        {
          peer_addr_size = sizeof(struct sockaddr_un);
          cfd = accept(sfd, (struct sockaddr*)&peer_addr, &peer_addr_size);
          if (cfd == -1)
          {
            LOG(ERROR, "could not accept: " << strerror(errno));
            close (sfd);
            unlink (m_path.c_str());
            return;
          }

          LOG(INFO, "new connection: " << cfd);

          /*
            TODO:
              - create process handling this fd
              - register process with the manager
              - handle messages in the manager
                   - manager forwards them to the corresponding place
                   - either segment_manager or gpi
                   - updates internal structures
                   - send back reply
          */

          const std::string msg ("hello\n");
          err = write (cfd, msg.c_str(), msg.size());
          if (err < 0)
          {
            LOG(ERROR, "could not write to client socket: " << strerror(errno));
            close(cfd);
            continue;
          }

          char buf [1024];
          memset (buf, 0, sizeof(buf));
          err = read (cfd, buf, sizeof(buf) - 1);
          if (err < 0)
          {
            LOG(ERROR, "could not read from client socket: " << strerror(errno));
            close (cfd);
            continue;
          }
          else
          {
            LOG(INFO, "got message (" << err << " bytes) : " << buf);
            if (buf[0] == 'q')
            {
              close (cfd);
              break;
            }
          }

          close (cfd);
        }

        close (sfd);
        unlink (m_path.c_str());
      }
    }
  }
}
