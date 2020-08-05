#pragma once

#include <we/type/value.hpp>

#include <string>
#include <list>

#include <boost/variant/variant_fwd.hpp>
#include <boost/variant/recursive_wrapper_fwd.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef pnet::type::value::value_type value_type;

      struct type;

      typedef std::list<std::string> path_type;
    }
  }
}
