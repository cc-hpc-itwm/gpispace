#pragma once

#include <we/type/value.hpp>

#include <list>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      std::list<std::pair<std::list<std::string>, value_type>>
        positions (value_type const&);
    }
  }
}
