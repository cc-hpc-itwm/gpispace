#pragma once

#include <we/type/signature.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef std::function< boost::optional<signature_type>
                               (const std::string&)
                           > resolver_type;

      signature_type resolve (const structured_type&, const resolver_type&);
    }
  }
}
