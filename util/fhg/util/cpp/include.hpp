// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_INCLUDE
#define _FHG_UTIL_CPP_INCLUDE 1

#include <fhg/util/ostream_modifier.hpp>

#include <boost/filesystem.hpp>

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      class include : public ostream::modifier
      {
      public:
        include (const std::string&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const std::string& _fname;
      };
    }
  }
}

#endif
