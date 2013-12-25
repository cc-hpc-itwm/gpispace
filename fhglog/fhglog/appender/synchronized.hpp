// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_SYNCHRONIZED_APPENDER_HPP
#define FHG_LOG_SYNCHRONIZED_APPENDER_HPP 1

#include <fhglog/Appender.hpp>
#include <boost/thread.hpp>

namespace fhg
{
  namespace log
  {
    class SynchronizedAppender : public Appender
    {
    public:
      explicit SynchronizedAppender (const Appender::ptr_t& appender)
        : _appender (appender)
      {}

      virtual void append (const LogEvent& event)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

        _appender->append (event);
      }

      virtual void flush()
      {
        boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

        _appender->flush();
      }

    private:
      mutable boost::recursive_mutex _mutex;
      Appender::ptr_t _appender;
    };
  }
}

#endif
