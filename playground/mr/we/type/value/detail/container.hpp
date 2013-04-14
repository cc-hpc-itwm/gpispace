// mirko.rahn@itwm.fraundhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_UTIL_CONTAINER_HPP
#define PNET_SRC_WE_TYPE_VALUE_UTIL_CONTAINER_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      typedef boost::variant< std::list<value_type>
                            , std::vector<value_type>
                            , std::set<value_type>
                            , std::map<value_type, value_type>
                            , structured_type
                            > value_container_type;
    }
  }
}

#endif
