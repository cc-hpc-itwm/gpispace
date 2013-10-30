// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_OSTREAM_MODIFIER_HPP
#define FHG_UTIL_OSTREAM_MODIFIER_HPP

#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class modifier
      {
      public:
        virtual std::ostream& operator() (std::ostream&) const = 0;
        virtual ~modifier() {}
      };
      std::ostream& operator<< (std::ostream&, const modifier&);
    }
  }
}

#endif
