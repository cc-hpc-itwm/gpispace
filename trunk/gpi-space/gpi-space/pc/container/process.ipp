/* -*- mode: c++ -*- */

#include "process.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
      template <typename M>
      gpi::pc::type::process_id_t process_t<M>::get_id () const
      {
        return m_id;
      }

      template <typename M>
      void process_t<M>::start ()
      {
        lock_type lock (m_mutex);
        if (! m_reader)
        {
          assert (m_socket >= 0);
          start_thread ();
        }
      }

      template <typename M>
      void process_t<M>::stop ()
      {
        lock_type lock (m_mutex);
        close_socket (m_socket);
        stop_thread ();
        m_socket = -1;
      }

      template <typename M>
      void process_t<M>::start_thread ()
      {
        assert (m_socket >= 0);
        assert (! m_reader);

        m_reader = thread_t
          (new boost::thread(boost::bind ( &self::reader_thread_main
                                         , this
                                         , m_socket
                                         )
                            )
          );
      }

      template <typename M>
      void process_t<M>::stop_thread ()
      {
        assert (m_reader);

        if (boost::this_thread::get_id() != m_reader->get_id())
        {
          m_reader->join ();
          m_reader.reset ();
        }
      }

      template <typename M>
      int process_t<M>::close_socket (const int fd)
      {
        shutdown (fd, SHUT_RDWR);
        return close (fd);
      }

      template <typename M>
      void process_t<M>::reader_thread_main(const int fd)
      {
        int err;
        char buf [2048];

        LOG(TRACE, "process container (" << m_id << ") started");
        for (;;)
        {
          memset (buf, 0, sizeof(buf));
          err = read (fd, buf, sizeof(buf) - 1);
          if (err < 0)
          {
            err = errno;
            LOG(ERROR, "could not read from client socket: " << strerror(err));
            close_socket (fd);
            m_mgr.handle_process_error (m_id, err);
            break;
          }
          else if (err == 0)
          {
            close_socket (fd);
            m_mgr.handle_process_error (m_id, err);
            break;
          }
          else
          {
            // just echo back for now
            buf[err] = 0;
            err = write (fd, buf, err);
            if (err < 0)
            {
              err = errno;
              LOG(ERROR, "could not write to client socket: " << strerror(err));
              break;
            }
          }
        }

        LOG(TRACE, "process container (" << m_id << ") terminated");
      }
    }
  }
}
