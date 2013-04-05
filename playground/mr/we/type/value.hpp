// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_HPP
#define PNET_SRC_WE_TYPE_VALUE_HPP

#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      typedef boost::make_recursive_variant
              < we::type::literal::control
              , bool
              , int
              , long
              , unsigned int
              , unsigned long
              , float
              , double
              , char
              , std::string
              , bitsetofint::type
              , bytearray::type
              , std::list<boost::recursive_variant_>
              , std::vector<boost::recursive_variant_>
              , std::set<boost::recursive_variant_>
              , std::map<boost::recursive_variant_, boost::recursive_variant_>
              , std::map<std::string, boost::recursive_variant_>
              >::type value_type;

      typedef std::map<std::string, value_type> structured_type;

      structured_type empty();
    }
  }
}

#endif
