#pragma once

#include <iml/util/parse/position.hpp>

#include <functional>
#include <string>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace parse
      {
        //! \note takeWhile (not . until), the character with until ==
        //! true is _not_ consumed, the same behaviour as
        //! HASKELL::Prelude::break:
        //! Prelude> break (=='f') "abcdefgh"
        //! ("abcde","fgh")

        std::string until
          (position&, std::function<bool (position const&)> const&);
      }
    }
  }
}