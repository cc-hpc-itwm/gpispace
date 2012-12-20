// mirko.rahn@itwm.fhg.de

#ifndef _WE_CONTAINER_IT_HPP
#define _WE_CONTAINER_IT_HPP

#include <we/util/it.hpp>

namespace we
{
  namespace container
  {
    template<typename Map, typename Res = typename Map::key_type>
    struct map_const_it : public it::it<typename Map::const_iterator>
    {
    public:
      explicit map_const_it (const Map& m)
        : map_const_it::super (m.begin(), m.end())
      {}

      const Res& operator* () const
      {
        return map_const_it::super::pos->first;
      }
    };

    template<typename Container>
    struct container_const_it
      : public it::it<typename Container::const_iterator>
    {
    public:
      explicit container_const_it (const Container& c)
        : container_const_it::super (c.begin(), c.end())
      {}

      const typename Container::value_type& operator* () const
      {
        return *container_const_it::super::pos;
      }
    };
  }
}

#endif
