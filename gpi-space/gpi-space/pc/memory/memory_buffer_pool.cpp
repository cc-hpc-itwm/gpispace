#include "memory_buffer_pool.hpp"

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
      buffer_pool_t::buffer_pool_t ()
        : m_min_size (std::numeric_limits<size_t>::max ())
        , m_max_size (0)
        , m_acquire_counter (0)
        , m_release_counter (0)
      {}

      buffer_pool_t::~buffer_pool_t ()
      {
        lock_type lock (m_mutex);

        MLOG_IF ( WARN
                , m_acquire_counter != m_release_counter
                , "destroying buffer pool, but not all buffers have been returned:"
                << " #acquired := " << m_acquire_counter
                << " #released := " << m_release_counter
                );

        while (not m_buffers.empty ())
        {
          buffer_t *buf = m_buffers.front (); m_buffers.pop_front ();
          delete buf;
        }
      }

      void buffer_pool_t::add_buffer (size_t sz)
      {
        lock_type lock (m_mutex);
        m_buffers.push_back (new buffer_t (sz));
        m_buffer_available.notify_one ();
      }

      buffer_t *buffer_pool_t::acquire ()
      {
        lock_type lock (m_mutex);
        while (m_buffers.empty ())
        {
          m_buffer_available.wait (lock);
        }

        buffer_t *buf = m_buffers.front (); m_buffers.pop_front ();
        ++m_acquire_counter;
        return buf;
      }

      void buffer_pool_t::release (buffer_t *buf)
      {
        lock_type lock (m_mutex);
        m_buffers.push_back (buf);
        ++m_release_counter;
        m_buffer_available.notify_one ();
      }
    }
  }
}
