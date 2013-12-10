// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_RESOLVE_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_RESOLVE_HPP

#include <we/type/signature.hpp>

#include <boost/function.hpp>
#include <boost/optional.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef boost::function< boost::optional<signature_type>
                                 (const std::string&)
                             > resolver_type;

      signature_type resolve (const structured_type&, const resolver_type&);
    }
  }
}

#endif
