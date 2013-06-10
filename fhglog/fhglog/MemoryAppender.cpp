#include "MemoryAppender.hpp"

namespace fhg
{
  namespace log
  {
    MemoryAppender::MemoryAppender (std::string const &name, size_t len)
      : Appender (name)
      , m_backlog_length (len)
    {}

    MemoryAppender::~MemoryAppender() throw ()
    {}

    void MemoryAppender::flush()
    {
      unique_lock lock (m_mutex);
      m_backlog.clear ();
    }

    void MemoryAppender::append(const LogEvent &evt)
    {
      unique_lock lock (m_mutex);
      m_backlog.push_front (evt);
      while (m_backlog.size () > m_backlog_length)
        m_backlog.pop_back ();
    }

    void MemoryAppender::set_backlog_length (size_t len)
    {
      unique_lock lock (m_mutex);
      m_backlog_length = len;
    }

    MemoryAppender::backlog_t MemoryAppender::backlog () const
    {
      shared_lock lock (m_mutex);
      return m_backlog;
    }

    MemoryAppender::ptr_t global_memory_appender (std::string const &name)
    {
      static MemoryAppender::ptr_t p (new MemoryAppender (name, 4096));
      return p;
    }
  }
}
