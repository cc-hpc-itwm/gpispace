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
              <std::list<std::pair< std::string
                                  , boost::variant< std::string
                                                  , boost::recursive_variant_
                                                  >
                                  >
                        >
              >::type signature_type;

      typedef std::list<std::pair< std::string
                                 , boost::variant< std::string
                                                 , signature_type
                                                 >
                                  >
                       > fields_type;
    }
  }
}

#endif
