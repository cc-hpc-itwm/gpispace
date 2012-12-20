// mirko.rahn@itwm.fhg.de

#ifndef _WE_CONTAINER_IT_HPP
#define _WE_CONTAINER_IT_HPP

#include <we/util/it.hpp>

namespace we
{
  namespace container
  {
    template<typename Map, typename Res = typename Map::key_type>
    struct const_it : public it::it<typename Map::const_iterator>
    {
    public:
      explicit const_it (const Map& m)
        : const_it::super (m.begin(), m.end())
      {}

      const Res& operator* () const
      {
        return const_it::super::pos->first;
      }
    };
  }
}

#endif
