#pragma once

#include <boost/variant.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef typename boost::make_recursive_variant
                       < std::pair<std::string, std::string>
                       , std::pair<std::string, std::list<boost::recursive_variant_>>
                       >::type field_type;

      typedef std::list<field_type> structure_type;

      typedef std::pair<std::string, structure_type> structured_type;

      typedef boost::variant<std::string, structured_type> signature_type;
    }
  }
}
