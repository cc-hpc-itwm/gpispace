// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_INCLUDE_GUARD
#define _FHG_UTIL_CPP_INCLUDE_GUARD 1

#include <iostream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      template<typename Stream>
      Stream& include_guard_begin (Stream& s, const std::string& what)
      {
        s << "#ifndef _" << what         << std::endl;
        s << "#define _" << what << " 1" << std::endl;
        s                                << std::endl;

        return s;
      }

      template<typename Stream>
      Stream& include_guard_end (Stream& s, const std::string & what)
      {
        return s << "#endif // _" << what << std::endl;
      }
    }
  }
}

#endif
