// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_PARSE_REQUIRE_HPP
#define FHG_UTIL_PARSE_REQUIRE_HPP

#include <fhg/util/parse/position.hpp>

#include <boost/function.hpp>

namespace fhg
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

        //! \note skip all white space characters
        void skip_spaces (position&);

        //! \note return the next character, throws when end()
        char plain_character (position&);

        //! \note return all characters until <until>, while
        //! 'escape''until' is ignored. 'until' will be consumed.
        std::string plain_string
          (position&, const char until, const char escape = '\\');

        //! \note a c-style identifier ([a-zA-Z_][a-zA-Z_0-9]*)
        std::string identifier (position&);

        //! \todo eliminate the skip_space in all the functions below

        //! \note skip spaces, require what
        void token (position&, const std::string& what);

        //! \note skip spaces, single-tick-character ('x')
        char character (position&);

        //! \note skip spaces, double-tick-string ("foobar")
        std::string string (position&);

        //! \note skip spaces, '0' or 'false' or 'no' or 'off' == false,
        //!                    '1' or 'on' or 'true' or 'yes' == true
        bool boolean (position&);

        //! \note expect a list, calling given function for all elements.
        //!       <open>[<skip? *><list_element><skip? *><sep>]*<close>
        //!       the function is expected to consume everything up to
        //!       the list seperator.
        void list ( position&
                  , const char open, const char sep, const char close
                  , const boost::function<void (position&)>&
                  , const bool skip_space_before_element = true
                  , const bool skip_space_after_element = true
                  );

        //! \note everything until pos.end()
        std::string rest (position&);
      }
    }
  }
}

#endif
