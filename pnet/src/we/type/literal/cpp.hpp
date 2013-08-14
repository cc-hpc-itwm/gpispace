// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_CPP_HPP
#define _WE_TYPE_LITERAL_CPP_HPP

#include <we/type/literal/name.hpp>

namespace literal
{
  namespace cpp
  {
    const std::string& translate (const std::string&);
    const std::string& include (const std::string&);

    bool reserved (const std::string&);
  }
}

#endif
