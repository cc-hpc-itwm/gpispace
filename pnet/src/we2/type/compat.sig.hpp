// mirko.rahn@itwm.fhg.de

#ifndef PNET_SRC_WE_TYPE_COMPAT_SIG_HPP
#define PNET_SRC_WE_TYPE_COMPAT_SIG_HPP

#include <we/type/signature.hpp>
#include <we2/type/signature.hpp>

#include <boost/optional.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace compat
    {
      signature::signature_type COMPAT
        ( const ::signature::type&
        , const boost::optional<std::string>& = boost::none
        );
    }
  }
}

#endif
