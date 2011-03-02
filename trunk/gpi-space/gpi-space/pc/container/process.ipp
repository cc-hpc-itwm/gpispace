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

#include <vector>

#include <fhglog/minimal.hpp>

#include <boost/bind.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/message_visitor.hpp>

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
      int process_t<M>::receive ( const int fd
                                , gpi::pc::proto::message_t & msg
                                , const size_t max_size
                                )
      {
        using namespace gpi::pc::proto;

        header_t header;
        int err;
        std::vector<char> buffer (1024);

        err = checked_read (fd, &header, sizeof(header));
        if (err <= 0)
        {
          return err;
        }

        if (header.version != 0x01)
        {
          LOG( ERROR
             , "invalid message received: version missmatch: "
             << "expected version: 0x" << std::hex << 0x01
             << " got 0x" << std::hex << (int)header.version
             );
          close_socket (fd);
          m_mgr.handle_process_error (m_id, EINVAL);
          return -EINVAL;
        }

        if (header.length > max_size)
        {
          LOG(ERROR, "message is larger than maximum allowed size (" << header.length << "), closing connection");
          close_socket (fd);
          m_mgr.handle_process_error (m_id, EMSGSIZE);
          return -EMSGSIZE;
        }

        try
        {
          buffer.resize (header.length);
        }
        catch (std::exception const &)
        {
          LOG(ERROR, "cannot accept new message: out of memory");
          m_mgr.handle_process_error (m_id, ENOMEM);
          return -ENOMEM;
        }

        err = checked_read (fd, &buffer[0], header.length);
        if (err <= 0)
        {
          return err;
        }

        try
        {
          decode_buffer (&buffer[0], header.length, msg);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not decode message: " << ex.what());
          m_mgr.handle_process_error (m_id, EINVAL);
          return -EINVAL;
        }

        return 1;
      }

      template <typename M>
      int process_t<M>::send (const int fd, gpi::pc::proto::message_t const & m)
      {
        using namespace gpi::pc::proto;

        int err;
        std::string data;
        header_t header;

        {
          std::stringstream sstr;
          boost::archive::text_oarchive oa (sstr);
          oa & m;
          data = sstr.str();
        }
        header.length = data.size();
        err = write (fd, &header, sizeof(header));
        if ( err <= 0 )
        {
          err = errno;
          m_mgr.handle_process_error (m_id, err);
          return -err;
        }

        err = write (fd, data.c_str(), header.length);
        if ( err <= 0 )
        {
          err = errno;
          m_mgr.handle_process_error (m_id, err);
          return -err;
        }

        return 1;
      }

      template <typename M>
      void process_t<M>::reader_thread_main(const int fd)
      {
        using namespace gpi::pc::proto;

        LOG(TRACE, "process container (" << m_id << ") started");
        for (;;)
        {
          try
          {
            message_t request;

            if (receive (fd, request) <= 0)
            {
              break;
            }

            message_t reply (handle_message (request));

            if (send (fd, reply) <= 0)
            {
              break;
            }
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "process container " << m_id << " crashed: " << ex.what());
            m_mgr.handle_process_error (m_id, EINVAL);
            break;
          }
        }

        LOG(TRACE, "process container (" << m_id << ") terminated");
      }

      template <typename M>
      void process_t<M>::decode_buffer (const char * buf, const size_t len, gpi::pc::proto::message_t & msg)
      {
        std::stringstream sstr (std::string (buf, len));
        boost::archive::text_iarchive ia (sstr);
        ia & msg;
      }

      template <typename M>
      int process_t<M>::checked_read (const int fd, void * buf, const size_t len)
      {
        int err;
        err = read (fd, buf, len);

        if (err < 0)
        {
          err = errno;
          LOG(ERROR, "could not read " << len << " bytes from client: " << strerror(err));
          close_socket (fd);
          m_mgr.handle_process_error (m_id, err);
          return -err;
        }
        else if (err == 0)
        {
          close_socket (fd);
          m_mgr.handle_process_error (m_id, err);
          return 0;
        }
        else
        {
          return err;
        }
      }

      template <typename M>
      gpi::pc::proto::message_t process_t<M>::handle_message(const gpi::pc::proto::message_t &msg)
      {
        using namespace gpi::pc::proto;

        typedef visitor::handle_message_t<self> message_handler_t;
        message_handler_t hdl (*this);
        return boost::apply_visitor (hdl, msg);
      }

      /********************************************************/
      /***                                                  ***/
      /***  P R O T O C O L    I M P L E M E N T A T I O N  ***/
      /***                                                  ***/
      /********************************************************/

      template <typename M>
      gpi::pc::type::handle_id_t
      process_t<M>::alloc ( const gpi::pc::type::segment_id_t seg
                          , const gpi::pc::type::size_t size
                          , const std::string & name
                          , const gpi::pc::type::flags_t flags
                          )
      {
        return m_mgr.alloc (m_id, seg, size, name, flags);
      }

      template <typename M>
      void process_t<M>::free (const gpi::pc::type::handle_id_t hdl)
      {
        return m_mgr.free (m_id, hdl);
      }

      template <typename M>
      void
      process_t<M>::list_allocations( const gpi::pc::type::segment_id_t seg
                                    , gpi::pc::type::handle::list_t & l
                                    ) const
      {
        m_mgr.list_allocations (m_id, seg, l);
      }

      template <typename M>
      gpi::pc::type::queue_id_t
      process_t<M>::memcpy ( gpi::pc::type::memory_location_t const & dst
                           , gpi::pc::type::memory_location_t const & src
                           , const gpi::pc::type::size_t amount
                           , const gpi::pc::type::queue_id_t queue
                           )
      {
        return m_mgr.memcpy (m_id, dst, src, amount, queue);
      }

      template <typename M>
      gpi::pc::type::size_t process_t<M>::wait (const gpi::pc::type::queue_id_t)
      {
        // this is not that easy to implement
        //    do we want to put the process container to sleep? - no
        //    we basically want to delay the answer
        //    the client should also be able to release the lock so that other threads can still interact with the pc
        //
        // TODO:
        //   client:
        //     attach a unique sequence number to the wait message
        //     enqueue the request
        //     send the message
        //     unlock communication lock
        //     wait for the request to "return"
        //
        //   server:
        //     enqueue the "wait" request into some queue that is handled by a seperate thread (each queue one thread)
        //     "reply" to the enqueued wait request via this process using the same sequence number waking up the client thread
        //
        //   implementation:
        //     processes' message visitor has to be rewritten to be "asynchronous" in some way
        //        -> idea: visitor's return value is a boost::optional
        //                 if set: reply immediately
        //                   else: somebody else will reply later
        //           messages need unique sequence numbers or message-ids
        return 0;
      }

      template <typename M>
      gpi::pc::type::segment_id_t
      process_t<M>::register_segment ( std::string const & name
                                     , const gpi::pc::type::size_t sz
                                     , const gpi::pc::type::flags_t flags
                                     )
      {
        gpi::pc::type::segment_id_t s_id
          (m_mgr.register_segment (m_id, name, sz, flags));
        return s_id;
      }

      template <typename M>
      void
      process_t<M>::unregister_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.unregister_segment (m_id, seg);
      }

      template <typename M>
      void
      process_t<M>::attach_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.attach_process_to_segment (m_id, seg);
      }

      template <typename M>
      void
      process_t<M>::detach_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.detach_process_from_segment (m_id, seg);
      }

      template <typename M>
      void
      process_t<M>::list_segments(gpi::pc::type::segment::list_t & l)
      {
        m_mgr.list_segments (m_id, l);
      }

      template <typename M>
      void process_t<M>::collect_info (gpi::pc::type::info::descriptor_t &d)
      {
        m_mgr.collect_info (d);
      }
    }
  }
}
