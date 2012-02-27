#include <fhglog/minimal.hpp>

#include "observable.hpp"
#include "observer.hpp"

#include <algorithm>

namespace observe
{
  namespace detail
  {
    void deliver_event_to(boost::any const & evt, const Observable* src, Observer *dst)
    {
      try
      {
        dst->notify(src, evt);
      }
      catch (std::exception const &ex)
      {
        LOG(WARN, "could not deliver event to " << dst);
      }
    }
  }

  void Observable::add_observer (Observer *o)
  {
    lock_type lock(m_mutex);
    if (std::find( m_observers.begin()
                 , m_observers.end()
                 , o
                 ) == m_observers.end()
       )
    {
      m_observers.push_back(o);
    }
  }

  void Observable::del_observer (Observer *o)
  {
    lock_type lock(m_mutex);
    m_observers.remove(o);
  }

  void Observable::emit(boost::any const & evt)
  {
    lock_type lock(m_mutex);
    std::for_each( m_observers.begin()
                 , m_observers.end()
                 , boost::bind( &detail::deliver_event_to
                              , boost::ref(evt)
                              , this
                              , _1
                              )
                );
  }
}
