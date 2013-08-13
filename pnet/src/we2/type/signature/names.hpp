// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_NAMES_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_NAMES_HPP

#include <we2/type/signature.hpp>

#include <boost/unordered_set.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      boost::unordered_set<std::string> names (const signature_type&);
    }
  }
}

#endif
