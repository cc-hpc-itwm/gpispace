#pragma once

#include <we/type/value.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      std::string to_string (value_type const&);
      value_type from_string (std::string const&);
    }
  }
}
