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

          gpi::pc::proto::message_t
          operator () (const gpi::pc::proto::memory::alloc_t & alloc)
          {
            try
            {
              /*
              gpi::pc::type::handle_id_t handle
                ( m_proc.alloc ( alloc.segment
                               , alloc.size
                               , alloc.flags
                               )
                );
              */
              throw std::runtime_error ("not yet implemented");
            }
            catch (std::exception const & ex)
            {
              gpi::pc::proto::error::error_t error;
              error.code = gpi::pc::proto::error::bad_request;
              error.detail = ex.what();
              return error;
            }
          }

          template <typename T>
          gpi::pc::proto::message_t
          operator () (T const &)
          {
            gpi::pc::proto::error::error_t error;
            error.code = gpi::pc::proto::error::bad_request;
            error.detail = "not understood input message";
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
