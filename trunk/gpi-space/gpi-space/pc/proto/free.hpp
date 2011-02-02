#ifndef GPI_SPACE_PC_PROTO_FREE_HPP
#define GPI_SPACE_PC_PROTO_FREE_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace free
      {
        struct request_t
        {
          gpi::pc::type::handle_id_t handle;
        };

        struct reply_t
        {
          // intentionally left blank for now
          // the information about the outcome is already handled within the error_t
        };
      }
    }
  }
}

#endif
