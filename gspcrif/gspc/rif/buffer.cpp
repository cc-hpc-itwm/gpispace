#include <algorithm>

#include "buffer.hpp"

namespace gspc
{
  namespace rif
  {
    buffer_t::buffer_t (size_t len)
      : m_data ()
    {
      m_data.reserve (len);
    }

    buffer_t::~buffer_t ()
    {}

    ssize_t buffer_t::read (char *dst, size_t len)
    {
      unique_lock lock (m_mutex);

      size_t s = std::min (m_data.size (), len);
      std::memcpy (dst, &m_data [0], s);
      m_data.erase (m_data.begin (), m_data.begin () + s);

      return s;
    }

    ssize_t buffer_t::write (const char *src, size_t len)
    {
      unique_lock lock (m_mutex);

      m_data.insert (m_data.end (), src, src+len);

      return len;
    }

    size_t buffer_t::size () const
    {
      shared_lock lock (m_mutex);
      return m_data.size ();
    }

    ssize_t buffer_t::write_to (int fd)
    {
      unique_lock lock (m_mutex);

      ssize_t s = ::write (fd, &m_data [0], m_data.size ());
      if (s > 0)
        m_data.erase (m_data.begin (), m_data.begin () + s);
      return s;
    }

    ssize_t buffer_t::read_from (int fd)
    {
      unique_lock lock (m_mutex);

      ssize_t count = 0;

      for (;;)
      {
        char c;
        ssize_t s = ::read (fd, &c, 1);
        if (s == 1)
        {
          m_data.push_back (c);
          count += s;
        }
        else
        {
          break;
        }
      }

      return count;
    }
  }
}
