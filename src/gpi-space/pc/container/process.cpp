#include <gpi-space/pc/container/process.hpp>

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
#include <fhg/syscall.hpp>
#include <fhg/util/nest_exceptions.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

#include <boost/variant/static_visitor.hpp>

#include <gpi-space/pc/memory/shm_area.hpp>

#include <gpi-space/pc/url.hpp>
#include <gpi-space/pc/url_io.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      namespace
      {
        struct handle_message_t : public boost::static_visitor<gpi::pc::proto::message_t>
        {
          handle_message_t ( gpi::pc::type::process_id_t const& proc_id
                           , memory::manager_t& memory_manager
                           , global::topology_t& topology
                           , gpi::api::gpi_api_t& gpi_api
                           )
            : m_proc_id (proc_id)
            , _memory_manager (memory_manager)
            , _topology (topology)
            , _gpi_api (gpi_api)
          {}

          /**********************************************/
          /***     M E M O R Y   R E L A T E D        ***/
          /**********************************************/

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::alloc_t & alloc) const
          {
            gpi::pc::proto::memory::alloc_reply_t rpl;
            rpl.handle = _memory_manager.alloc
              ( m_proc_id
              , alloc.segment, alloc.size, alloc.name, alloc.flags
              );
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::free_t & free) const
          {
            _memory_manager.free (free.handle);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::memcpy_t & cpy) const
          {
            gpi::pc::type::validate (cpy.dst.handle);
            gpi::pc::type::validate (cpy.src.handle);
            gpi::pc::proto::memory::memcpy_reply_t rpl;
            rpl.queue = _memory_manager.memcpy
              (cpy.dst, cpy.src, cpy.size, cpy.queue);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::wait_t & w) const
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
            rpl.count = _memory_manager.wait_on_queue
              (m_proc_id, w.queue);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::list_t & list) const
          {
            gpi::pc::proto::memory::list_reply_t rpl;
            if (list.segment == gpi::pc::type::segment::SEG_INVAL)
            {
              _memory_manager.list_allocations(m_proc_id, rpl.list);
            }
            else
            {
              _memory_manager.list_allocations (m_proc_id, list.segment, rpl.list);
            }
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::memory::info_t & info) const
          {
            gpi::pc::proto::memory::info_reply_t rpl;
            rpl.descriptor = _memory_manager.info (info.handle);
            return gpi::pc::proto::memory::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator() (const gpi::pc::proto::memory::get_transfer_costs_t& request) const
          {
            return gpi::pc::proto::memory::message_t
              (gpi::pc::proto::memory::transfer_costs_t (_memory_manager.get_transfer_costs (request.transfers)));
          }

          /**********************************************/
          /***     S E G M E N T   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::register_t & register_segment) const
          {
            url_t url ("shm", register_segment.name);
            url.set ("size", boost::lexical_cast<std::string>(register_segment.size));
            if (register_segment.flags & F_PERSISTENT)
              url.set ("persistent", "true");
            if (register_segment.flags & F_EXCLUSIVE)
              url.set ("exclusive", "true");

            memory::area_ptr_t area (memory::shm_area_t::create (boost::lexical_cast<std::string>(url), _memory_manager.handle_generator()));
            area->set_owner (m_proc_id);

            gpi::pc::proto::segment::register_reply_t rpl;
            rpl.id =
              _memory_manager.register_memory (m_proc_id, area);

            return gpi::pc::proto::segment::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::unregister_t & unregister_segment) const
          {
            _memory_manager.unregister_memory
              (m_proc_id, unregister_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::attach_t & attach_segment) const
          {
            _memory_manager.attach_process
              (m_proc_id, attach_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::detach_t & detach_segment) const
          {
            _memory_manager.detach_process
              (m_proc_id, detach_segment.id);
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::success, "success");
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::list_t & list) const
          {
            gpi::pc::proto::segment::list_reply_t rpl;
            if (list.id == gpi::pc::type::segment::SEG_INVAL)
              _memory_manager.list_memory (rpl.list);
            else
            {
              LOG(WARN, "list of particular segment not implemented");
              _memory_manager.list_memory (rpl.list);
            }
            return gpi::pc::proto::segment::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::add_memory_t & add_mem) const
          {
            gpi::pc::type::segment_id_t id =
              _memory_manager.add_memory (m_proc_id, add_mem.url, 0, _topology);
            gpi::pc::proto::segment::register_reply_t rpl;
            rpl.id = id;
            return gpi::pc::proto::segment::message_t (rpl);
          }

          gpi::pc::proto::message_t
            operator () (const gpi::pc::proto::segment::del_memory_t & del_mem) const
          {
            _memory_manager.del_memory (m_proc_id, del_mem.id, _topology);
            return
              gpi::pc::proto::error::error_t (gpi::pc::proto::error::success);
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
            rpl.info.rank = _gpi_api.rank();
            rpl.info.nodes = _gpi_api.number_of_nodes();
            rpl.info.queues = _gpi_api.number_of_queues();
            rpl.info.queue_depth = _gpi_api.queue_depth();
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
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::bad_request,  "invalid input message");
          }

        private:
          gpi::pc::type::process_id_t const& m_proc_id;
          memory::manager_t& _memory_manager;
          global::topology_t& _topology;
          gpi::api::gpi_api_t& _gpi_api;
        };

        gpi::pc::proto::message_t handle_message
          ( gpi::pc::type::process_id_t const& id
          , gpi::pc::proto::message_t const& request
          , gpi::pc::memory::manager_t& memory_manager
          , global::topology_t& topology
          , gpi::api::gpi_api_t& gpi_api
          )
        {
          try
          {
            return boost::apply_visitor
              (handle_message_t (id, memory_manager, topology, gpi_api), request);
          }
          catch (std::exception const& ex)
          {
            return gpi::pc::proto::error::error_t
              (gpi::pc::proto::error::bad_request, ex.what());
          }
        }
      }

      process_t::~process_t()
      {
        try
        {
          fhg::syscall::shutdown (m_socket, SHUT_RDWR);
        }
        catch (boost::system::system_error const& se)
        {
          if (se.code() != boost::system::errc::not_connected)
          {
            //! \note ignored: already disconnected = fine, we terminate
            throw;
          }
        }
        fhg::syscall::close (m_socket);

        if (boost::this_thread::get_id() != m_reader.get_id())
        {
          if (m_reader.joinable())
          {
            m_reader.join ();
          }
        }
      }

      namespace
      {
        void read_exact (int fd, void* buffer, ssize_t size)
        {
          if (fhg::syscall::read (fd, buffer, size) != size)
          {
            throw std::runtime_error
              ("unable to read " + std::to_string (size) + " bytes");
          }
        }
        void write_exact (int fd, void const* buffer, ssize_t size)
        {
          if (fhg::syscall::write (fd, buffer, size) != size)
          {
            throw std::runtime_error
              ("unable to write " + std::to_string (size) + " bytes");
          }
        }
      }

      void process_t::reader_thread_main()
      {
        LOG(TRACE, "process container (" << m_id << ") started on socket " << m_socket);

        for (;;)
        {
          try
          {
            gpi::pc::proto::message_t request;

            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  gpi::pc::proto::header_t header;
                  read_exact (m_socket, &header, sizeof (header));

                  std::vector<char> buffer (header.length);
                  read_exact (m_socket, buffer.data(), buffer.size());

                  fhg::util::nest_exceptions<std::runtime_error>
                    ( [&]
                      {
                        std::stringstream sstr
                          (std::string (buffer.data(), buffer.size()));
                        boost::archive::binary_iarchive ia (sstr);
                        ia & request;
                      }
                    , "could not decode message"
                    );
                }
              , "could not receive message"
              );

            gpi::pc::proto::message_t const reply
              (handle_message (m_id, request, _memory_manager, _topology, _gpi_api));

            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  std::string data;
                  gpi::pc::proto::header_t header;

                  fhg::util::nest_exceptions<std::runtime_error>
                    ( [&]
                      {
                        std::stringstream sstr;
                        boost::archive::text_oarchive oa (sstr);
                        oa & reply;
                        data = sstr.str();
                      }
                    , "could not encode message"
                    );

                  header.length = data.size();

                  write_exact (m_socket, &header, sizeof (header));
                  write_exact (m_socket, data.data(), data.size());
                }
              , "could not send message"
              );
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "process container " << m_id << " crashed: " << ex.what());
            m_handle_process_error (m_id, EINVAL);
            break;
          }
        }

        LOG(TRACE, "process container (" << m_id << ") terminated");
      }
    }
  }
}
