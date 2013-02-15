// mirko.rahn@itwm.fraunhofer.de

#include <we/container/priostore.hpp>

#include <algorithm>

namespace we
{
  namespace container
  {
    namespace
    {
      using petri_net::transition_id_type;

      typedef std::vector<transition_id_type> vec_type;
      typedef std::pair<vec_type::iterator,vec_type::iterator> pit_type;

      void vec_insert (vec_type& v, const transition_id_type& x)
      {
        const pit_type pit (std::equal_range (v.begin(), v.end(), x));

        if (pit.first == pit.second)
        {
          v.insert (pit.second, x);
        }
      }

      void vec_erase (vec_type& v, const transition_id_type& x)
      {
        const pit_type pit (std::equal_range (v.begin(), v.end(), x));

        if (pit.first != pit.second)
        {
          v.erase (pit.first);
        }
      }

      bool vec_elem (const vec_type& v, const transition_id_type& x)
      {
        return std::binary_search (v.begin(), v.end(), x);
      }
    }

    void priority_store::erase ( const petri_net::transition_id_type& x
                               , const prio_map_t::iterator& pos
                               )
    {
      if (pos != _prio_map.end())
      {
        vec_erase (pos->second, x);

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
      vec_insert (_prio_map[prio], x);
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

      return (pos != _prio_map.end()) ? vec_elem (pos->second, x) : false;
    }

    bool priority_store::empty() const
    {
      return _prio_map.empty();
    }
  }
}
