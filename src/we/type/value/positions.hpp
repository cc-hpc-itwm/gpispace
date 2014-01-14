// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_POSITIONS_HPP
#define PNET_SRC_WE_TYPE_VALUE_POSITIONS_HPP

#include <we/type/value.hpp>

#include <list>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      std::list<std::pair<std::list<std::string>, value_type> >
        positions (value_type const&);
    }
  }
}

#endif
