// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_PUT_HPP
#define PNET_SRC_WE_TYPE_VALUE_PUT_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type put ( const std::list<std::string>& path
                     , const value_type& value
                     , const value_type& node
                     );
    }
  }
}

#endif
