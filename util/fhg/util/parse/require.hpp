// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_PARSE_REQUIRE_HPP
#define FHG_UTIL_PARSE_REQUIRE_HPP

#include <fhg/util/parse/position.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace require
      {
        //! \note return the next character, throws when end()
        char plain_character (position&);

        //! \todo eliminate the skip_space in all the functions below

        //! \note skip spaces, require what
        void token (fhg::util::parse::position&, const std::string& what);

        //! \note skip spaces, single-tick-character ('x')
        char character (fhg::util::parse::position&);

        //! \note skip spaces, double-tick-string ("foobar")
        std::string string (fhg::util::parse::position&);

        //! \note skip spaces, '0' or 'false' or 'no' or 'off' == false,
        //!                    '1' or 'on' or 'true' or 'yes' == true
        bool boolean (fhg::util::parse::position&);
      }
    }
  }
}

#endif
