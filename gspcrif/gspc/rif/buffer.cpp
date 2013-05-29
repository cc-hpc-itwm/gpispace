#include <algorithm>

#include "buffer.hpp"

namespace gspc
{
  namespace rif
  {
    buffer_t::buffer_t (size_t len)
      : m_data (len)
    {
      m_data.linearize ();
    }

    buffer_t::~buffer_t ()
    {}

    ssize_t buffer_t::read (char *dst, size_t len)
    {
      unique_lock lock (m_mutex);

      size_t s = std::min (m_data.size (), len);
      for (size_t i = 0 ; i < s ; ++i)
      {
        dst [i] = m_data.front (); m_data.pop_front ();
      }

      return s;
    }

    ssize_t buffer_t::write (const char *src, size_t len)
    {
      unique_lock lock (m_mutex);

      for (size_t i = 0 ; i < len ; ++i)
      {
        m_data.push_back (src [i]);
      }

      return len;
    }

    size_t buffer_t::size () const
    {
      shared_lock lock (m_mutex);
      return m_data.size ();
    }

    size_t buffer_t::capacity () const
    {
      shared_lock lock (m_mutex);
      return m_data.capacity ();
    }

    ssize_t buffer_t::write_to (int fd)
    {
      unique_lock lock (m_mutex);
      char *buf = m_data.linearize ();

      ssize_t s = ::write (fd, buf, m_data.size ());
      if (s > 0)
        m_data.erase (m_data.begin (), m_data.begin () + s);
      return s;
    }

    ssize_t buffer_t::read_from (int fd)
    {
      unique_lock lock (m_mutex);

      ssize_t len = m_data.capacity ();
      ssize_t count = 0;

      while (len > 0)
      {
        char c;
        ssize_t s = ::read (fd, &c, 1);
        if (s > 0)
        {
          m_data.push_back (c);
          ++count;
          --len;
        }
        else
        {
          break;
        }
      }

      return (ssize_t)count;
    }
  }
}
