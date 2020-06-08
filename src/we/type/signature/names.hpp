#pragma once

#include <we/type/signature.hpp>

#include <string>
#include <unordered_set>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      std::unordered_set<std::string> names (const signature_type&);
    }
  }
}
