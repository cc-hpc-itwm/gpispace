#include "memory_buffer_pool.hpp"

#include <limits>
#include <algorithm> // std::find

#include <fhg/assert.hpp>
#include <fhglog/fhglog.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      template <typename B>
      buffer_pool_t<B>::buffer_pool_t ()
        : m_min_size (std::numeric_limits<size_t>::max ())
        , m_max_size (0)
        , m_acquire_counter (0)
        , m_release_counter (0)
      {}

      template <typename B>
      buffer_pool_t<B>::~buffer_pool_t ()
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
          buffer_type *buf = m_buffers.front (); m_buffers.pop_front ();
          delete buf;
        }
      }

      template <typename B>
      void buffer_pool_t<B>::add (buffer_pool_t::buffer_type *buf)
      {
        lock_type lock (m_mutex);
        m_buffers.push_back (buf);
        m_buffer_available.notify_one ();
      }

      template <typename B>
      void buffer_pool_t<B>::del (buffer_pool_t::buffer_type **buf_p)
      {
        fhg_assert (buf_p);

        typename buffer_pool_t::buffer_type *buf = *buf_p;

        lock_type lock (m_mutex);
        typename buffer_pool_t::buffer_list_t::iterator it =
          std::find (m_buffers.begin (), m_buffers.end (), buf);
        if (it == m_buffers.end ())
        {
          throw std::runtime_error
            ("buffer is not registered or still in use!");
        }
        m_buffers.erase (it);
        delete buf;
        *buf_p = 0;
      }

      template <typename B>
      typename buffer_pool_t<B>::buffer_type *buffer_pool_t<B>::acquire ()
      {
        lock_type lock (m_mutex);
        while (m_buffers.empty ())
        {
          m_buffer_available.wait (lock);
        }

        typename buffer_pool_t<B>::buffer_type *buf =
          m_buffers.front (); m_buffers.pop_front ();
        ++m_acquire_counter;
        return buf;
      }

      template <typename B>
      void buffer_pool_t<B>::release (buffer_pool_t::buffer_type *buf)
      {
        lock_type lock (m_mutex);
        m_buffers.push_back (buf);
        ++m_release_counter;
        m_buffer_available.notify_one ();
      }
    }
  }
}
