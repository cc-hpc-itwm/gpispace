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

#include <gpi-space/pc/proto/message.hpp>

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
        using namespace gpi::pc::proto;

        int err;
        char buf [4096];

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
            message_t * msg = (message_t*)(&buf);
            try
            {
              handle_message (fd, msg);
            }
            catch (std::exception const & ex)
            {
              LOG( ERROR, "could not handle incomming message: "
                 << "pc = " << m_id << " "
                 << "mc = " << msg->type << " "
                 << "err = " << ex.what()
                 );
              break;
            }
          }
        }

        LOG(TRACE, "process container (" << m_id << ") terminated");
      }

      template <typename M>
      void process_t<M>::handle_message( const int fd
                                       , const gpi::pc::proto::message_t *msg
                                       )
      {
        using namespace gpi::pc::proto;

        int err;
        char buf [4096];
        memset (buf, 0, sizeof(buf));

        message_t * rpl = (message_t*)(&buf);

        switch (msg->type)
        {
        case message::memory_alloc:
          {
            LOG(INFO, *msg->as<memory::alloc_t>());
            rpl->type = message::error;
            rpl->length = sizeof(error::error_t);
            error::error_t *err_msg = rpl->as<error::error_t>();
            err_msg->code = error::out_of_memory;
            break;
          }
        default:
          {
            LOG(WARN, "bad request: " << msg->type);
            rpl->type = message::error;
            rpl->length = sizeof(error::error_t);
            error::error_t *err_msg = rpl->as<error::error_t>();
            err_msg->code = error::bad_request;
            break;
          }
        }

        err = write (fd, rpl, sizeof(message_t) + rpl->length);

        if (err < 0)
        {
          err = errno;
          LOG(ERROR, "could not write to client socket: " << strerror(err));
          throw std::runtime_error ("could not write to client socket: " + std::string(strerror(err)));
        }
      }
    }
  }
}
