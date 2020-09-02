#pragma once

#include <iml/util/parse/position.hpp>

#include <string>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace parse
      {
        namespace require
        {
          //! \note require the given value or throw
          void require (position&, const char&);
          void require (position&, const std::string&);

          //! \note a c-style identifier ([a-zA-Z_][a-zA-Z_0-9]*)
          std::string identifier (position&);
        }
      }
    }
  }
}
