#ifndef FHG_LOG_MEMORY_APPENDER_HPP
#define FHG_LOG_MEMORY_APPENDER_HPP 1

#include <deque>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class MemoryAppender : public Appender
  {
  public:
    typedef shared_ptr<MemoryAppender> ptr_t;
    typedef std::deque<LogEvent> backlog_t;

    explicit
    MemoryAppender (std::string const &name, size_t backlog);

    virtual ~MemoryAppender() throw();

    virtual void flush();
    virtual void append (const LogEvent &evt);

    backlog_t backlog () const;

    void set_backlog_length (size_t n);
  private:
    typedef boost::shared_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> unique_lock;
    typedef boost::shared_lock<mutex_type> shared_lock;

    mutable mutex_type m_mutex;

    size_t m_backlog_length;
    backlog_t m_backlog;
  };

    MemoryAppender::ptr_t global_memory_appender (std::string const &name);
}}

#endif
