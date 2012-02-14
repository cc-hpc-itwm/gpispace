/*
 * =====================================================================================
 *
 *       Filename:  SynchronizedAppender.hpp
 *
 *    Description:  synchronizes an Appender
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

#ifndef FHG_LOG_SYNCHRONIZED_APPENDER_HPP
#define FHG_LOG_SYNCHRONIZED_APPENDER_HPP 1

#include <fhglog/DecoratingAppender.hpp>
#include <boost/thread.hpp>

namespace fhg { namespace log {
  class SynchronizedAppender : public DecoratingAppender
  {
  private:
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

  public:
    explicit
    SynchronizedAppender(const Appender::ptr_t &appender)
      : DecoratingAppender(appender, "-sync")
    {}

    explicit
    SynchronizedAppender(Appender *appender)
      : DecoratingAppender(appender, "-sync")
    {}

    virtual void append(const LogEvent &evt)
    {
      lock_type lock (m_mutex);
      DecoratingAppender::append(evt);
    }

    virtual void flush(void)
    {
      lock_type lock (m_mutex);
      DecoratingAppender::flush();
    }
  private:
    mutable mutex_type m_mutex;
  };
}}

#endif
