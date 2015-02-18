// mirko.rahn@itwm.fraunhofer.de

#pragma once

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
        virtual ~modifier() = default;
      };
      std::ostream& operator<< (std::ostream&, const modifier&);
    }
  }
}
