// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_PARSE_FROM_STRING_HPP
#define _FHG_UTIL_PARSE_FROM_STRING_HPP

#include <fhg/util/parse/position.hpp>

#include <boost/function.hpp>

#include <string>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      template<typename T>
        T from_string ( boost::function <T (position&)> read
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

#endif
