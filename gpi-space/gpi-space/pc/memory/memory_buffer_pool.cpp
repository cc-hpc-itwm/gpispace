#include "memory_buffer_pool.ipp"

#include <limits>

#include <fhg/assert.hpp>
#include <fhglog/fhglog.hpp>

#include "memory_buffer.hpp"
#include "handle_buffer.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      template class buffer_pool_t<buffer_t>;
      template class buffer_pool_t<handle_buffer_t>;
    }
  }
}
