// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_PUT_HPP
#define _WE_TYPE_VALUE_PUT_HPP

#include <we/type/value.hpp>

namespace value
{
  void put ( path_type::const_iterator, const path_type::const_iterator
           , type&, const type&
           );
  void put (const path_type&, type&, const type&);
  void put (const std::string&, type&, const type&);
}

#endif
