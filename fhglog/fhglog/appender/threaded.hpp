// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_THREADED_APPENDER_HPP
#define FHG_LOG_THREADED_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <deque>

namespace fhg
{
  namespace log
  {
    class ThreadedAppender : public Appender
    {
    public:
      typedef boost::shared_ptr<ThreadedAppender> ptr_t;

      explicit
      ThreadedAppender (const Appender::ptr_t &appender)
        : _appender (appender)
      {
        start();
      }

      ~ThreadedAppender()
      {
        stop();
      }

      void start()
      {
        if (_log_thread.get_id() != boost::thread::id())
        {
          return;
        }

        _log_thread =
          boost::thread (boost::bind ( &ThreadedAppender::log_thread_loop
                                     , this
                                     )
                        );
      }

      void stop()
      {
        if (_log_thread.get_id() == boost::thread::id())
        {
          return;
        }

        _log_thread.interrupt();
        _log_thread.join();
      }

      virtual void append (const LogEvent& event)
      {
        boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

        _events.push_back (event);
        _event_available.notify_one();
      }

      virtual void flush()
      {
        boost::unique_lock<boost::recursive_mutex> lock (_mutex);

        while (!_events.empty())
        {
          _flushed.wait (lock);
        }

        _appender->flush();
      }

    private:
      void log_thread_loop()
      {
        while (true)
        {
          boost::unique_lock<boost::recursive_mutex> lock (_mutex);

          while (_events.empty())
          {
            _event_available.wait (lock);
          }

          _appender->append (_events.front());

          _events.pop_front();

          if (_events.empty())
          {
            _flushed.notify_all();
          }
        }
      }

    private:
      Appender::ptr_t _appender;
      boost::thread _log_thread;
      boost::recursive_mutex _mutex;
      boost::condition_variable_any _event_available;
      boost::condition_variable_any _flushed;
      std::deque<LogEvent> _events;
    };
  }
}

#endif
