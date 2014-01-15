// mirko.rahn@itwm.fraunhofer.de

#include <we/container/priostore.hpp>

namespace we
{
  namespace container
  {
    void priority_store::insert (const we::transition_id_type& x, we::priority_type priority)
    {
      _prio_map[priority].insert (x);
    }

    void priority_store::erase (const we::transition_id_type& x, we::priority_type priority)
    {
      prio_map_t::iterator const pos (_prio_map.find (priority));

      if (pos != _prio_map.end())
      {
        pos->second.erase (x);

        if (pos->second.empty())
        {
          _prio_map.erase (pos);
        }
      }
    }

    bool priority_store::elem (const we::transition_id_type& x, we::priority_type priority) const
    {
      const prio_map_t::const_iterator pos (_prio_map.find (priority));

      return (pos != _prio_map.end())
        && pos->second.find (x) != pos->second.end();
    }

    bool priority_store::empty() const
    {
      return _prio_map.empty();
    }
  }
}
