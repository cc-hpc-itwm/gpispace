#include "memory_buffer_pool.ipp"

#include <limits>

#include <fhg/assert.hpp>
#include <fhglog/fhglog.hpp>

#include "memory_buffer.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      template class buffer_pool_t<buffer_t>;
    }
  }
}
