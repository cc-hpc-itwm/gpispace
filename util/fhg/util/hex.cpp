#include "hex.hpp"

#include <boost/foreach.hpp>

#include <cctype>
#include <iomanip>
#include <stdexcept>
#include <sstream>

namespace fhg
{
  namespace util
  {
    std::string to_hex (std::string const &s)
    {
      std::ostringstream sstr;

      BOOST_FOREACH (char const c, s)
      {
        sstr << std::setw (2)
             << std::setfill ('0')
             << std::hex
             << (int)(c & 0xff)
          ;
      }

      return sstr.str ();
    }

    namespace
    {
    char from_hex (char c)
    {
      switch (tolower (c))
      {
      case '0'...'9':
        return 0 +  (c - '0');
      case 'a'...'f':
        return 10 + (c - 'a');
      default:
        throw std::invalid_argument ("from_hex: not a hexadecimal character");
      }
    }
    }

    std::string from_hex (std::string const &s)
    {
      std::string retval;

      if (s.size () % 2 != 0)
        throw std::invalid_argument ("from_hex: string length must be even");

      std::string::const_iterator c = s.begin ();
      const std::string::const_iterator end = s.end ();
      for ( ; c != end ; c += 2)
      {
        retval += (from_hex (*c) << 4) | from_hex (*(c+1));
      }

      return retval;
    }
  }
}
