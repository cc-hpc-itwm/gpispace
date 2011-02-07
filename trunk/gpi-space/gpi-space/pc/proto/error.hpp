#ifndef GPI_SPACE_PC_PROTO_ERROR_HPP
#define GPI_SPACE_PC_PROTO_ERROR_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace error
      {
        enum errc
          {
            success = 0,
            bad_request = 10,
            out_of_memory = 30,
          };

        struct error_t
        {
          errc code;
        };

        typedef error_t message_t;
      }
    }
  }
}

#endif
