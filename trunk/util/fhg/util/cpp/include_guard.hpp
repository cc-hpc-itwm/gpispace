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
      inline std::ostream & include_guard_begin ( std::ostream & os
                                                , const std::string & what
                                                )
      {
        os << "#ifndef _" << what         << std::endl;
        os << "#define _" << what << " 1" << std::endl;
        os                                << std::endl;

        return os;
      }

      inline std::ostream & include_guard_end ( std::ostream & os
                                              , const std::string & what
                                              )
      {
        os << "#endif // _" << what << std::endl;

        return os;
      }
    }
  }
}

#endif
