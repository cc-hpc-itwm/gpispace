// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_TYPE_HPP
#define _WE_TYPE_VALUE_CONTAINER_TYPE_HPP 1

#include <boost/unordered_map.hpp>

#include <we/type/value.hpp>

#include <list>

namespace value
{
  namespace container
  {
    typedef boost::unordered_map<std::string, value::type> type;

    typedef std::list<std::string> key_vec_t;
  }
}

#endif
