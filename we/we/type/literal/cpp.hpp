// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_CPP_HPP
#define _WE_TYPE_LITERAL_CPP_HPP

#include <we/type/literal/name.hpp>

namespace literal
{
  namespace cpp
  {
    const std::string& translate (const type_name_t&);
    const std::string& include (const type_name_t&);

    bool known (const type_name_t&);
    bool reserved (const std::string&);
  }
}

#endif
