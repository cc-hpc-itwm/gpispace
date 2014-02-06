// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_PARSE_UNTIL_HPP
#define FHG_UTIL_PARSE_UNTIL_HPP

#include <fhg/util/parse/position.hpp>

#include <boost/function.hpp>

namespace fhg
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
        (position&, boost::function<bool (position const&)> const&);
    }
  }
}

#endif
