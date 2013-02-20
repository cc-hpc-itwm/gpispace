// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_SHOW_HPP
#define _WE_TYPE_VALUE_SHOW_HPP

#include <we/type/value.hpp>

#include <iosfwd>

namespace value
{
  std::ostream& operator<< (std::ostream&, const type&);
}

#endif
