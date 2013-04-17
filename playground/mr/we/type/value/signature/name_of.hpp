// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SIGNATURE_NAME_OF_HPP
#define PNET_SRC_WE_TYPE_VALUE_SIGNATURE_NAME_OF_HPP

#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T> const std::string& name_of (const T&);
    }
  }
}

#endif
