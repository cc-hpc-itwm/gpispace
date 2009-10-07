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

#include <pthread.h>
#include <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class SynchronizedAppender : public Appender
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
      : Appender(appender->name() + "-sync")
      , real_appender_(appender)
    {
      pthread_mutex_init(&mtx_, NULL);
    }

    explicit
    SynchronizedAppender(Appender *appender)
      : Appender(appender->name() + "-sync")
      , real_appender_(appender)
    {
      pthread_mutex_init(&mtx_, NULL);
    }

    ~SynchronizedAppender()
    {
      pthread_mutex_destroy(&mtx_);
    }

    virtual inline void setFormat(Formatter *fmt)
    {
      real_appender_->setFormat(fmt);
    }
    virtual inline void setFormat(const Formatter::ptr_t &fmt)
    {
      real_appender_->setFormat(fmt);
    }
    virtual inline const Formatter::ptr_t &getFormat() const
    {
      return real_appender_->getFormat();
    }

    virtual void append(const LogEvent &evt) const
    {
      Lock lock(&mtx_);
      real_appender_->append(evt);
    }
  private:
    Appender::ptr_t real_appender_;
    pthread_mutex_t mtx_;
  };
}}
