#include <gpi-space/pc/memory/memory_buffer.hpp>

#include <fhg/assert.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      buffer_t::buffer_t (std::vector<char>::size_type sz)
        : m_data (sz)
        , m_used (0)
      {
        fhg_assert (sz > 0, "buffer size must be positive");
      }
    }
  }
}