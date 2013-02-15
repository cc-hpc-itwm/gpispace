// mirko.rahn@itwm.fraunhofer.de

#include <we/container/priostore.hpp>

namespace we
{
  namespace container
  {
    void priority_store::erase ( const petri_net::transition_id_type& x
                               , const prio_map_t::iterator& pos
                               )
    {
      if (pos != _prio_map.end())
      {
        pos->second.erase (x);

        if (pos->second.empty())
        {
          _prio_map.erase (pos);
        }
      }
    }

    void priority_store::insert ( const petri_net::transition_id_type& x
                                , const petri_net::priority_type& prio
                                )
    {
      _prio_map[prio].insert(x);
    }

    petri_net::priority_type
    priority_store::get_priority (const petri_net::transition_id_type& x) const
    {
      const get_prio_t::const_iterator pos (_get_prio.find (x));

      return (pos == _get_prio.end())
        ? petri_net::priority_type() : pos->second;
    }

    void priority_store::set_priority ( const petri_net::transition_id_type& x
                                      , const petri_net::priority_type& prio
                                      )
    {
      const bool is_elem (elem (x));

      if (is_elem)
      {
        erase (x);
      }

      _get_prio[x] = prio;

      if (is_elem)
      {
        insert (x, prio);
      }
    }

    void priority_store::erase_priority (const petri_net::transition_id_type& x)
    {
      const bool is_elem (elem (x));

      if (elem (x))
      {
        erase (x);
      }

      _get_prio.erase (x);

      if (is_elem)
      {
        insert (x);
      }
    }

    void priority_store::insert (const petri_net::transition_id_type& x)
    {
      insert (x, get_priority (x));
    }

    void priority_store::erase (const petri_net::transition_id_type& x)
    {
      erase (x, _prio_map.find (get_priority (x)));
    }

    bool priority_store::elem (const petri_net::transition_id_type& x) const
    {
      const prio_map_t::const_iterator pos (_prio_map.find (get_priority (x)));

      return (pos != _prio_map.end()) ? pos->second.elem (x) : false;
    }

    bool priority_store::empty() const
    {
      return _prio_map.empty();
    }
  }
}
