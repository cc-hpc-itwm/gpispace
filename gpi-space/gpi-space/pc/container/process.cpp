#include "process.hpp"

#include <gpi-space/gpi/api.hpp>

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

#include <gpi-space/pc/memory/factory.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

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
          handle_message_t (gpi::pc::type::process_id_t const& proc_id)
            : m_proc_id (proc_id)
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
                ( global::memory_manager().alloc
                  ( m_proc_id
                  , alloc.segment, alloc.size, alloc.name, alloc.flags
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
              global::memory_manager().free (free.handle);
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
              gpi::pc::type::validate (cpy.dst.handle);
              gpi::pc::type::validate (cpy.src.handle);
              gpi::pc::proto::memory::memcpy_reply_t rpl;
              rpl.queue = global::memory_manager().memcpy
                (m_proc_id, cpy.dst, cpy.src, cpy.size, cpy.queue);
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
              rpl.count = global::memory_manager().wait_on_queue
                (m_proc_id, w.queue);
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
              if (list.segment == gpi::pc::type::segment::SEG_INVAL)
              {
                global::memory_manager().list_allocations(m_proc_id, rpl.list);
              }
              else
              {
                global::memory_manager().list_allocations (m_proc_id, list.segment, rpl.list);
              }
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
              rpl.descriptor = global::memory_manager().info (info.handle);
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
              fhg::util::url_t url;
              url.type ("shm");
              url.path (register_segment.name);
              url.set ("size", boost::lexical_cast<std::string>(register_segment.size));
              if (register_segment.flags & F_PERSISTENT)
                url.set ("persistent", "true");
              if (register_segment.flags & F_EXCLUSIVE)
                url.set ("exclusive", "true");

              memory::area_ptr_t area =
                memory::factory ().create (boost::lexical_cast<std::string>(url));
              area->set_owner (m_proc_id);

              gpi::pc::proto::segment::register_reply_t rpl;
              rpl.id =
                global::memory_manager().register_memory (m_proc_id, area);

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
              global::memory_manager().unregister_memory
                (m_proc_id, unregister_segment.id);
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
              global::memory_manager().attach_process
                (m_proc_id, attach_segment.id);
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
              global::memory_manager().detach_process
                (m_proc_id, detach_segment.id);
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
                global::memory_manager().list_memory (rpl.list);
              else
              {
                LOG(WARN, "list of particular segment not implemented");
                global::memory_manager().list_memory (rpl.list);
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
                global::memory_manager ().add_memory (m_proc_id, add_mem.url);
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
              global::memory_manager ().del_memory (m_proc_id, del_mem.id);
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
            gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());

            rpl.info.rank = gpi_api.rank();
            rpl.info.nodes = gpi_api.number_of_nodes();
            rpl.info.queues = gpi_api.number_of_queues();
            rpl.info.queue_depth = gpi_api.queue_depth();
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
          gpi::pc::type::process_id_t const& m_proc_id;
        };
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
        gpi::pc::proto::header_t header;

        {
        int const err (checked_read (fd, &header, sizeof(header)));
        if (err <= 0)
        {
          return err;
        }
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

        std::vector<char> buffer;

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

        {
        int const err (checked_read (fd, &buffer[0], header.length));
        if (err <= 0)
        {
          return err;
        }
        }

        try
        {
          std::stringstream sstr (std::string (&buffer[0], header.length));
          boost::archive::binary_iarchive ia (sstr);
          ia & msg;
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
                     , boost::apply_visitor ( visitor::handle_message_t (m_id)
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
    }
  }
}
