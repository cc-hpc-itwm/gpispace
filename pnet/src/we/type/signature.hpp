// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_HPP

#include <boost/variant.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef boost::make_recursive_variant
              < std::pair
                < std::string
                , std::list< boost::variant< std::pair<std::string, std::string>
                                           , boost::recursive_variant_
                                           >
                           >
                >
              >::type structured_type;

      typedef boost::variant< std::pair<std::string, std::string>
                            , structured_type
                            > field_type;

      typedef std::list<field_type> structure_type;

      typedef boost::variant<std::string, structured_type> signature_type;
    }
  }
}

#endif
