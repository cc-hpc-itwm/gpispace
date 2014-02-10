#ifndef SDPA_PLUGIN_OBSERVABLE_HPP
#define SDPA_PLUGIN_OBSERVABLE_HPP 1

#include <list>
#include <boost/any.hpp>
#include <boost/thread.hpp>

namespace observe
{
  class Observer;
  class Observable
  {
    typedef boost::mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    typedef std::list<Observer*> observer_list_t;
  public:
    void emit (boost::any const &);

    void add_observer(Observer *);
    void del_observer(Observer *);
  private:
    mutable mutex_type m_mutex;
    observer_list_t m_observers;
  };
}

#endif
