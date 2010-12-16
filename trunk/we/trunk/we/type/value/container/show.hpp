// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_SHOW_HPP
#define _WE_TYPE_VALUE_CONTAINER_SHOW_HPP 1

#include <we/type/value/container/type.hpp>

#include <we/type/value/show.hpp>

#include <iostream>

namespace value
{
  namespace container
  {
    inline void show (std::ostream & s, const type & t)
    {
      for (type::const_iterator pos (t.begin()); pos != t.end(); ++pos)
        {
          s << pos->first << " := " << pos->second << std::endl;
        }
    }
  }
}

#endif
