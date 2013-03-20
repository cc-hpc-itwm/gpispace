/*
 * =====================================================================================
 *
 *       Filename:  ThreadedAppender.hpp
 *
 *    Description:  uses a thread to decouple the real logging effort from the
 *					application code
 *
 *        Version:  1.0
 *        Created:  10/07/2009 11:36:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_THREADED_APPENDER_HPP
#define FHG_LOG_THREADED_APPENDER_HPP 1

#include <boost/thread.hpp>
#include <deque>
#include <fhglog/DecoratingAppender.hpp>

#ifndef NDEBUG
#	include <iostream>
#endif

namespace fhg { namespace log {
  class ThreadedAppender : public DecoratingAppender
  {
  private:
	typedef std::deque<LogEvent> event_list_type;
	typedef boost::recursive_mutex mutex_type;
	typedef boost::unique_lock<mutex_type> lock_type;
	typedef boost::condition_variable_any condition_type;

  public:
	typedef shared_ptr<ThreadedAppender> ptr_t;

    explicit
    ThreadedAppender(const Appender::ptr_t &appender)
      : DecoratingAppender(appender, "-thrd")
    {
	  start();
	}

    explicit
    ThreadedAppender(Appender *appender)
      : DecoratingAppender(appender, "-thrd")
    {
	  start();
    }

    ~ThreadedAppender()
    {
	  try
	  {
		stop();
	  }
	  catch (const std::exception &ex)
	  {
#ifndef NDEBUG
		std::clog << "E: error during stop of ThreadedAppender: " << ex.what() << std::endl;
#endif
	  }
	  catch (...)
	  {
#ifndef NDEBUG
		std::clog << "E: unknown error during stop of ThreadedAppender!" << std::endl;
#endif
	  }
    }

	void start()
	{
	  if (log_thread_.get_id() != boost::thread::id()) return;

	  log_thread_ = boost::thread(boost::bind(&ThreadedAppender::log_thread_loop, this));
	}

	void stop()
	{
	  if (log_thread_.get_id() == boost::thread::id()) return;

	  log_thread_.interrupt();
	  log_thread_.join();
	}

    virtual void append(const LogEvent &evt)
    {
	  lock_type lock(mtx_);
	  events_.push_back(evt);
	  event_available_.notify_one();
    }

	virtual void flush() throw (boost::thread_interrupted)
	{
	  lock_type lock(mtx_);
	  while (! events_.empty())
	  {
		flushed_.wait(lock);
	  }
          DecoratingAppender::flush ();
	}
  private:
	void log_thread_loop()
	{
	  for (;;)
	  {
            lock_type lock(mtx_);
            while (events_.empty())
            {
              event_available_.wait(lock);
            }
            LogEvent evt = events_.front(); events_.pop_front();
            DecoratingAppender::append(evt);

            if (events_.empty())
            {
              flushed_.notify_all();
            }
          }
        }

	private:
	  boost::thread log_thread_;
	  mutex_type mtx_;
	  condition_type event_available_;
	  condition_type flushed_;
	  event_list_type events_;
  };
}}

#endif
