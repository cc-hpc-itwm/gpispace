// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_RANDOM_STRING_HPP
#define FHG_UTIL_RANDOM_STRING_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    // uniformly select a char from chars
    char random_char_of (std::string const& chars);

    // random string of random length <= 2^10, character uniform from chars
    std::string random_string_of (std::string const& chars);

    // equivalent to random_string_of (map char [0..255])
    std::string random_string();

    // equivalent to
    // random_string_of (filter (not . elem except) . map char [0..255])
    std::string random_string_without (std::string const& except);

    // [a-zA-Z_][a-zA-Z_0-9]*
    std::string random_identifier();

    // content of xml tags, e.g. [^\0<>\\]
    std::string random_content_string();

    //! \note that boost::format as well as std::string::operator+ have
    //! problems with the zero
    std::string random_string_without_zero();
  }
}

#endif
