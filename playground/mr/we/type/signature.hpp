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
      typedef boost::make_recursive_variant
              < std::string
              , std::list<boost::recursive_variant_>
              , std::set<boost::recursive_variant_>
              , std::map<boost::recursive_variant_, boost::recursive_variant_>
              , std::pair< std::string
                         , std::map<std::string, boost::recursive_variant_>
                         >
              >::type signature_type;

      typedef std::map<std::string, signature_type> structured_type;
      typedef std::pair<std::string, structured_type> field_type;
    }
  }
}

#endif
