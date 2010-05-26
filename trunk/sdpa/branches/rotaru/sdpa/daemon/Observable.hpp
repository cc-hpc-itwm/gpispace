/*
 * =====================================================================================
 *
 *       Filename:  Observable.hpp
 *
 *    Description:  the Observable part of the observer pattern
 *
 *        Version:  1.0
 *        Created:  11/19/2009 10:45:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_OBSERVABLE_HPP
#define SDPA_DAEMON_OBSERVABLE_HPP 1

#include <list>
#include <stdexcept>
#include <boost/thread.hpp>
#include <sdpa/daemon/Observer.hpp>

namespace sdpa { namespace daemon {
  class Observable
  {
    typedef boost::recursive_mutex mutex_t;
    typedef boost::unique_lock<mutex_t> lock_t;
  public:
    void attach_observer(Observer *observer)
    {
      lock_t lock(mtx_);
      for (observer_list_t::const_iterator o(observers_.begin()); o != observers_.end(); ++o)
      {
        if (*o == observer)
          return; // already registered
      }
      observers_.push_back(observer);
    }

    void detach_observer(Observer *observer)
    {
      lock_t lock(mtx_);
      observers_.remove(observer);
    }

    virtual ~Observable() {}

  protected:
    void notifyObservers(const boost::any &event) const
    {
      lock_t lock(const_cast<mutex_t&>(mtx_));
      for (observer_list_t::const_iterator o(observers_.begin()); o != observers_.end(); ++o)
      {
        try
        {
          (*o)->update(event);
        }
        catch (const std::exception & ex)
        {
          std::cerr << "E: exception during notification handling: " << ex.what() << std::endl;
        }
      }
    }
  private:
    typedef std::list<Observer*> observer_list_t;
    observer_list_t observers_;
    mutex_t mtx_;
  };
}}

#endif
