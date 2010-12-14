// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_INCLUDE
#define _FHG_UTIL_CPP_INCLUDE 1

#include <iostream>
#include <string>

#include <fhg/util/cpp/types.hpp>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      inline std::ostream & include ( std::ostream & os
                                    , const std::string & what
                                    )
      {
        if (what.size() > 0)
          {
            os << "#include <" << what << ">" << std::endl;
          }

        return os;
      }

      inline std::ostream & include ( std::ostream & os
                                    , const char * what
                                    )
      {
        return include (os, std::string (what));
      }

      inline std::ostream & include ( std::ostream & os
                                    , const path_type & file
                                    )
      {
        return include (os, file.string());
      }
    }
  }
}

#endif
