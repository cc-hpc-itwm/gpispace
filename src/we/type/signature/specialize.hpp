// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_SPECIALIZE_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_SPECIALIZE_HPP

#include <we/type/signature.hpp>

#include <string>
#include <unordered_map>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      void specialize ( structured_type&
                      , const std::unordered_map<std::string, std::string>&
                      );
    }
  }
}

#endif
