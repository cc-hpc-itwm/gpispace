#pragma once

#include <fhg/util/parse/position.hpp>

#include <functional>
#include <string>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      template<typename T>
        T from_string ( std::function <T (position&)> read
                      , std::string const& input
                      )
      {
        position_string pos (input);

        T const x (read (pos));

        if (!pos.end())
        {
          throw std::runtime_error (pos.error_message ("additional input"));
        }

        return x;
      }
    }
  }
}
