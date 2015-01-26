// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_CPP_INCLUDE_GUARD
#define _FHG_UTIL_CPP_INCLUDE_GUARD 1

#include <fhg/util/ostream_modifier.hpp>

#include <iosfwd>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace include_guard
      {
        class open : public ostream::modifier
        {
        public:
          open (const std::string&);
          virtual std::ostream& operator() (std::ostream&) const override;
        private:
          const std::string _name;
        };

        class close : public ostream::modifier
        {
        public:
          virtual std::ostream& operator() (std::ostream&) const override;
        };
      }
    }
  }
}

#endif
