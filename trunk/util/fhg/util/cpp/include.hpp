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
      template<typename Stream>
      Stream& include (Stream& s, const std::string& what)
      {
        if (what.size() > 0)
          {
            s << "#include <" << what << ">" << std::endl;
          }

        return s;
      }

      template<typename Stream>
      Stream& include (Stream& s, const char* what)
      {
        return include (s, std::string (what));
      }

      template<typename Stream>
      Stream& include (Stream& s, const path_type& file)
      {
        return include (s, file.string());
      }
    }
  }
}

#endif
