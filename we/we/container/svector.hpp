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

    std::vector<petri_net::transition_id_type> vec;

  public:
    void insert (const petri_net::transition_id_type& x)
    {
      const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

      if (pit.first == pit.second)
      {
        vec.insert (pit.second, x);
      }
    }

    void erase (const petri_net::transition_id_type& x)
    {
      const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

      if (pit.first != pit.second)
      {
        vec.erase (pit.first);
      }
    }

    bool elem (const petri_net::transition_id_type& x) const
    {
      return std::binary_search (vec.begin(), vec.end(), x);
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
