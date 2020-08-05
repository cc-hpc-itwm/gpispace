#pragma once

#include <we/type/signature.hpp>

#include <string>
#include <unordered_map>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      void specialize ( structured_type&
                      , const std::unordered_map<std::string, std::string>&
                      );
    }
  }
}
