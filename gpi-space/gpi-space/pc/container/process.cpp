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

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <boost/bind.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

#include <boost/variant/static_visitor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      namespace visitor
      {
        struct handle_message_t : public boost::static_visitor<gpi::pc::proto::message_t>
        {
          handle_message_t (process_t & proc)
            : m_proc (proc)
          {}

          /**********************************************/
          /***     M E M O R Y   R E L A T E D        ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::alloc_t & alloc) const
          {
            try
            {
              gpi::pc::type::handle_id_t handle
                ( m_proc.alloc ( alloc.segment
                               , alloc.size
                               , alloc.name
                               , alloc.flags
                               )
                );
              gpi::pc::proto::memory::alloc_reply_t rpl;
              rpl.handle = handle;
              return gpi::pc::proto::memory::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              LOG(ERROR, "allocation of " << alloc.size << " bytes in segment " << alloc.segment << " failed: " << ex.what());
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::free_t & free) const
          {
            try
            {
              m_proc.free (free.handle);
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::success;
              error.detail = "success";
              return error;
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::memcpy_t & cpy) const
          {
            try
            {
              gpi::pc::proto::memory::memcpy_reply_t rpl;
              rpl.queue = m_proc.memcpy ( cpy.dst
                                        , cpy.src
                                        , cpy.size
                                        , cpy.queue
                                        );
              return gpi::pc::proto::memory::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::wait_t & w) const
          {
            try
            {
              gpi::pc::proto::memory::wait_reply_t rpl;
              rpl.count = m_proc.wait (w.queue);
              return gpi::pc::proto::memory::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::list_t & list) const
          {
            try
            {
              gpi::pc::proto::memory::list_reply_t rpl;
              m_proc.list_allocations (list.segment, rpl.list);
              return gpi::pc::proto::memory::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::info_t & info) const
          {
            try
            {
              gpi::pc::proto::memory::info_reply_t rpl;
              rpl.descriptor = m_proc.info (info.handle);
              return gpi::pc::proto::memory::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          /**********************************************/
          /***     S E G M E N T   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::register_t & register_segment) const
          {
            try
            {
              gpi::pc::type::segment_id_t id =
                m_proc.register_segment ( register_segment.name
                                        , register_segment.size
                                        , register_segment.flags
                                        );
              gpi::pc::proto::segment::register_reply_t rpl;
              rpl.id = id;
              return gpi::pc::proto::segment::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::unregister_t & unregister_segment) const
          {
            try
            {
              m_proc.unregister_segment(unregister_segment.id);
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::success;
              error.detail = "success";
              return error;
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::attach_t & attach_segment) const
          {
            try
            {
              m_proc.attach_segment (attach_segment.id);
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::success;
              error.detail = "success";
              return error;
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::detach_t & detach_segment) const
          {
            try
            {
              m_proc.detach_segment (detach_segment.id);
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::success;
              error.detail = "success";
              return error;
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::list_t & list) const
          {
            try
            {
              gpi::pc::proto::segment::list_reply_t rpl;
              if (list.id == gpi::pc::type::segment::SEG_INVAL)
                m_proc.list_segments (rpl.list);
              else
              {
                LOG(WARN, "list of particular segment not implemented");
                m_proc.list_segments (rpl.list);
              }
              return gpi::pc::proto::segment::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::add_memory_t & add_mem) const
          {
            try
            {
              gpi::pc::type::segment_id_t id =
                m_proc.add_memory (add_mem.url);
              gpi::pc::proto::segment::register_reply_t rpl;
              rpl.id = id;
              return gpi::pc::proto::segment::message_t (rpl);
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::del_memory_t & del_mem) const
          {
            try
            {
              m_proc.del_memory (del_mem.id);
              return
                gpi::pc::proto::error::error_t (gpi::pc::proto::error::success);
            }
            catch (std::exception const & ex)
            {
              return gpi::pc::proto::error::error_t
                (gpi::pc::proto::error::bad_request, ex.what());
            }
          }

          /**********************************************/
          /***     C O N T R O L   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::ping_t &) const
          {
            gpi::pc::proto::control::pong_t pong;
            return gpi::pc::proto::control::message_t (pong);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::info_t &) const
          {
            gpi::pc::proto::control::info_reply_t rpl;
            m_proc.collect_info (rpl.info);
            return gpi::pc::proto::control::message_t (rpl);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::message_t & m) const
          {
            return boost::apply_visitor (*this, m);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::message_t & m) const
          {
            return boost::apply_visitor (*this, m);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::message_t & m) const
          {
            return boost::apply_visitor (*this, m);
          }

          /*** Catch all other messages ***/

          template <typename T>
          gpi::pc::proto::message_t
          operator () (T const &) const
          {
            gpi::pc::proto::error::error_t error;
            error.code = gpi::pc::proto::error::bad_request;
            error.detail = "invalid input message";
            return error;
          }

        private:
          process_t & m_proc;
        };
      }
    }
  }
}

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      gpi::pc::type::process_id_t process_t::get_id () const
      {
        return m_id;
      }

      void process_t::start ()
      {
        lock_type lock (m_mutex);
        if (! m_reader)
        {
          assert (m_socket >= 0);
          start_thread ();
        }
      }

      void process_t::stop ()
      {
        lock_type lock (m_mutex);
        close_socket (m_socket);
        stop_thread ();
        m_socket = -1;
      }

      void process_t::start_thread ()
      {
        assert (m_socket >= 0);
        assert (! m_reader);

        m_reader = thread_t
          (new boost::thread(boost::bind ( &process_t::reader_thread_main
                                         , this
                                         , m_socket
                                         )
                            )
          );
      }

      void process_t::stop_thread ()
      {
        assert (m_reader);

        if (boost::this_thread::get_id() != m_reader->get_id())
        {
          m_reader->join ();
          m_reader.reset ();
        }
      }

      int process_t::close_socket (const int fd)
      {
        shutdown (fd, SHUT_RDWR);
        return close (fd);
      }

      int process_t::receive ( const int fd
                                , gpi::pc::proto::message_t & msg
                                , const size_t max_size
                                )
      {
        using namespace gpi::pc::proto;

        header_t header;
        int err;
        std::vector<char> buffer;

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

      int process_t::send (const int fd, gpi::pc::proto::message_t const & m)
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

      void process_t::reader_thread_main(const int fd)
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

            if (send ( fd
                     , boost::apply_visitor ( visitor::handle_message_t (*this)
                                            , request
                                            )
                     ) <= 0
               )
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

      void process_t::decode_buffer (const char * buf, const size_t len, gpi::pc::proto::message_t & msg)
      {
        std::stringstream sstr (std::string (buf, len));
        boost::archive::binary_iarchive ia (sstr);
        ia & msg;
      }

      int process_t::checked_read (const int fd, void * buf, const size_t len)
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

      /********************************************************/
      /***                                                  ***/
      /***  P R O T O C O L    I M P L E M E N T A T I O N  ***/
      /***                                                  ***/
      /********************************************************/

      gpi::pc::type::handle_id_t
      process_t::alloc ( const gpi::pc::type::segment_id_t seg
                          , const gpi::pc::type::size_t size
                          , const std::string & name
                          , const gpi::pc::type::flags_t flags
                          )
      {
        return m_mgr.alloc (m_id, seg, size, name, flags);
      }

      void process_t::free (const gpi::pc::type::handle_id_t hdl)
      {
        return m_mgr.free (m_id, hdl);
      }

      gpi::pc::type::handle::descriptor_t
      process_t::info (const gpi::pc::type::handle_id_t hdl) const
      {
        return m_mgr.info (m_id, hdl);
      }

      void
      process_t::list_allocations( const gpi::pc::type::segment_id_t seg
                                    , gpi::pc::type::handle::list_t & l
                                    ) const
      {
        m_mgr.list_allocations (m_id, seg, l);
      }

      gpi::pc::type::queue_id_t
      process_t::memcpy ( gpi::pc::type::memory_location_t const & dst
                           , gpi::pc::type::memory_location_t const & src
                           , const gpi::pc::type::size_t amount
                           , const gpi::pc::type::queue_id_t queue
                           )
      {
        return m_mgr.memcpy (m_id, dst, src, amount, queue);
      }

      gpi::pc::type::size_t process_t::wait (const gpi::pc::type::queue_id_t queue)
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
        return m_mgr.wait_on_queue (m_id, queue);
      }

      gpi::pc::type::segment_id_t
      process_t::register_segment ( std::string const & name
                                     , const gpi::pc::type::size_t sz
                                     , const gpi::pc::type::flags_t flags
                                     )
      {
        gpi::pc::type::segment_id_t s_id
          (m_mgr.register_segment (m_id, name, sz, flags));
        return s_id;
      }

      void
      process_t::unregister_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.unregister_segment (m_id, seg);
      }

      gpi::pc::type::segment_id_t
      process_t::add_memory (std::string const &url)
      {
        return m_mgr.add_memory (m_id, url);
      }

      void
      process_t::del_memory (gpi::pc::type::segment_id_t seg_id)
      {
        m_mgr.del_memory (m_id, seg_id);
      }

      void
      process_t::attach_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.attach_process_to_segment (m_id, seg);
      }

      void
      process_t::detach_segment(const gpi::pc::type::segment_id_t seg)
      {
        m_mgr.detach_process_from_segment (m_id, seg);
      }

      void
      process_t::list_segments(gpi::pc::type::segment::list_t & l)
      {
        m_mgr.list_segments (m_id, l);
      }

      void process_t::collect_info (gpi::pc::type::info::descriptor_t &d)
      {
        m_mgr.collect_info (d);
      }
    }
  }
}
