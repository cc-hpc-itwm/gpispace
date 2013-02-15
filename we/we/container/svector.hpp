// set with access to nth element, mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_CONTAINER_SVECTOR_HPP
#define _WE_CONTAINER_SVECTOR_HPP

#include <we/type/id.hpp>

#include <vector>

#include <algorithm>

#include <boost/random.hpp>

namespace we
{
  namespace container
  {
    struct svector
    {
    public:
      void insert (const petri_net::transition_id_type& x)
      {
        const pit_t pit (std::equal_range (_vec.begin(), _vec.end(), x));

        if (pit.first == pit.second)
        {
          _vec.insert (pit.second, x);
        }
      }

      void erase (const petri_net::transition_id_type& x)
      {
        const pit_t pit (std::equal_range (_vec.begin(), _vec.end(), x));

        if (pit.first != pit.second)
        {
          _vec.erase (pit.first);
        }
      }

      bool elem (const petri_net::transition_id_type& x) const
      {
        return std::binary_search (_vec.begin(), _vec.end(), x);
      }

      bool empty() const
      {
        return _vec.empty();
      }

      template<typename Engine>
      const petri_net::transition_id_type& random (Engine& engine) const
      {
        boost::uniform_int<std::size_t> rand (0, _vec.size()-1);
        return _vec.at (rand (engine));
      }

    private:
      typedef std::vector<petri_net::transition_id_type>::iterator it_type;
      typedef std::pair<it_type,it_type> pit_t;

      std::vector<petri_net::transition_id_type> _vec;
    };
  }
}

#endif
