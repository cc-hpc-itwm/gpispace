#include "observable.hpp"
#include "observer.hpp"

#include <algorithm>

namespace observe
{
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
                 , boost::bind( &Observer::notify
                              , _1
                              , evt
                              )
                );
  }
}
