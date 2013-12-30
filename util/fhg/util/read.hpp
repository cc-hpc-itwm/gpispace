// {petry,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_HPP
#define _FHG_UTIL_READ_HPP 1

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    inline int read_int (const std::string& showed)
    {
      parse::position_string pos (showed);

      return read_int (pos);
    }
    inline size_t read_size_t (const std::string& showed)
    {
      parse::position_string pos (showed);

      return read_uint (pos);
    }
  }
}

#endif
