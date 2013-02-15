// set with access to nth element, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_SVECTOR_HPP
#define _CONTAINER_SVECTOR_HPP

#include <we/type/id.hpp>

#include <vector>

#include <algorithm>

#include <boost/random.hpp>

namespace svector
{
  struct type
  {
  private:
    typedef std::vector<petri_net::transition_id_type>::iterator it_type;
    typedef std::pair<it_type,it_type> pit_t;
    typedef std::vector<petri_net::transition_id_type>::const_iterator const_it;

    std::vector<petri_net::transition_id_type> vec;

    pit_t lookup (const petri_net::transition_id_type& x)
    {
      return std::equal_range (vec.begin(), vec.end(), x);
    }

    std::pair<const_it,const_it>
    lookup (const petri_net::transition_id_type& x) const
    {
      return std::equal_range (vec.begin(), vec.end(), x);
    }

    template<typename PIT>
    bool member (const PIT& pit) const
    {
      return std::distance (pit.first, pit.second) > 0;
    }

  public:
    void insert (const petri_net::transition_id_type& x)
    {
      const pit_t pit (lookup (x));

      if (!member (pit))
        vec.insert (pit.second, x);
    }

    void erase (const petri_net::transition_id_type& x)
    {
      const pit_t pit (lookup (x));

      if (member (pit))
        vec.erase (pit.first);
    }

    bool elem (const petri_net::transition_id_type& x) const
    {
      return member (lookup (x));
    }

    template<typename Engine>
    const petri_net::transition_id_type& random (Engine& engine) const
    {
      boost::uniform_int<std::size_t> rand (0, vec.size()-1);
      return vec.at (rand (engine));
    }

    bool empty() const
    {
      return vec.empty();
    }
  };
}

#endif
