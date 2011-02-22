#ifndef GPI_SPACE_PC_CONTAINER_MESSAGE_VISITOR_HPP
#define GPI_SPACE_PC_CONTAINER_MESSAGE_VISITOR_HPP 1

#include <boost/variant/static_visitor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      namespace visitor
      {
        template <typename P>
        struct handle_message_t : public boost::static_visitor<gpi::pc::proto::message_t>
        {
          typedef P process_type;

          handle_message_t (process_type & proc)
            : m_proc (proc)
          {}

          /**********************************************/
          /***     M E M O R Y   R E L A T E D        ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::alloc_t & alloc)
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
          operator () (const gpi::pc::proto::memory::free_t & free)
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
          operator () (const gpi::pc::proto::memory::memcpy_t & cpy)
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
          operator () (const gpi::pc::proto::memory::wait_t & w)
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
          operator () (const gpi::pc::proto::memory::list_t & list)
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

          /**********************************************/
          /***     S E G M E N T   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::register_t & register_segment)
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
          operator () (const gpi::pc::proto::segment::unregister_t & unregister_segment)
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
          operator () (const gpi::pc::proto::segment::attach_t & attach_segment)
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
          operator () (const gpi::pc::proto::segment::detach_t & detach_segment)
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
          operator () (const gpi::pc::proto::segment::list_t & list)
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


          /**********************************************/
          /***     C O N T R O L   R E L A T E D      ***/
          /**********************************************/

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::ping_t &)
          {
            gpi::pc::proto::control::pong_t pong;
            return gpi::pc::proto::control::message_t (pong);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::info_t &)
          {
            gpi::pc::proto::control::info_reply_t rpl;
            m_proc.collect_info (rpl.info);
            return gpi::pc::proto::control::message_t (rpl);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::segment::message_t & m)
          {
            return boost::apply_visitor (*this, m);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::message_t & m)
          {
            return boost::apply_visitor (*this, m);
          }

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::control::message_t & m)
          {
            return boost::apply_visitor (*this, m);
          }

          /*** Catch all other messages ***/

          template <typename T>
          gpi::pc::proto::message_t
          operator () (T const &)
          {
            gpi::pc::proto::error::error_t error;
            error.code = gpi::pc::proto::error::bad_request;
            error.detail = "invalid input message";
            return error;
          }

        private:
          process_type & m_proc;
        };
      }
    }
  }
}

#endif
