// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_HPP

#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef std::pair<std::string, std::size_t> name_with_position_type;

      typedef boost::make_recursive_variant
              < std::map<name_with_position_type, std::string>
              , std::map<name_with_position_type, boost::recursive_variant_>
              >::type signature_type;
    }
  }
}

#endif
