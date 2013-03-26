// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_FIND_HPP
#define _WE_TYPE_VALUE_FIND_HPP

#include <we/type/value.hpp>

#include <list>

namespace value
{
  const value::type& find ( std::list<std::string>::const_iterator
                          , const std::list<std::string>::const_iterator
                          , const value::type&
                          );
}

#endif
