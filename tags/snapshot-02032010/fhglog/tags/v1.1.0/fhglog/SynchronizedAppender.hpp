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

#include <pthread.h>
#include <fhglog/DecoratingAppender.hpp>

namespace fhg { namespace log {
  class SynchronizedAppender : public DecoratingAppender
  {
  private:
    class Lock
    {
    public:
      explicit
      Lock(const pthread_mutex_t *lock)
        : lock_(lock)
      {
        pthread_mutex_lock(const_cast<pthread_mutex_t*>(lock_));
      }

      ~Lock()
      {
        pthread_mutex_unlock(const_cast<pthread_mutex_t*>(lock_));
      }

      private:
        const pthread_mutex_t *lock_;
    };

  public:
    explicit
    SynchronizedAppender(const Appender::ptr_t &appender)
      : DecoratingAppender(appender, "-sync")
    {
      pthread_mutex_init(&mtx_, NULL);
    }

    explicit
    SynchronizedAppender(Appender *appender)
      : DecoratingAppender(appender, "-sync")
    {
      pthread_mutex_init(&mtx_, NULL);
    }

    ~SynchronizedAppender()
    {
      pthread_mutex_destroy(&mtx_);
    }

    virtual void append(const LogEvent &evt) const
    {
      Lock lock(&mtx_);
      DecoratingAppender::append(evt);
    }
  private:
    pthread_mutex_t mtx_;
  };
}}

#endif
