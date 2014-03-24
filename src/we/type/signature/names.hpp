// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_NAMES_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_NAMES_HPP

#include <we/type/signature.hpp>

#include <string>
#include <unordered_set>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      std::unordered_set<std::string> names (const signature_type&);
    }
  }
}

#endif
