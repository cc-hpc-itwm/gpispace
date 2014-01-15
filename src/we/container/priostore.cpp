// mirko.rahn@itwm.fraunhofer.de

#include <we/container/priostore.hpp>

#include <algorithm>

namespace we
{
  namespace container
  {
    namespace
    {
      using we::transition_id_type;

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

    void priority_store::insert (const we::transition_id_type& x, we::priority_type priority)
    {
      vec_insert (_prio_map[priority], x);
    }

    void priority_store::erase (const we::transition_id_type& x, we::priority_type priority)
    {
      prio_map_t::iterator const pos (_prio_map.find (priority));

      if (pos != _prio_map.end())
      {
        vec_erase (pos->second, x);

        if (pos->second.empty())
        {
          _prio_map.erase (pos);
        }
      }
    }

    bool priority_store::elem (const we::transition_id_type& x, we::priority_type priority) const
    {
      const prio_map_t::const_iterator pos (_prio_map.find (priority));

      return (pos != _prio_map.end()) && vec_elem (pos->second, x);
    }

    bool priority_store::empty() const
    {
      return _prio_map.empty();
    }
  }
}
