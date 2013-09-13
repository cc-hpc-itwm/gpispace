// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/ostream_modifier.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      modifier::~modifier() {}

      std::ostream& operator<< (std::ostream& os, const modifier& m)
      {
        return m (os);
      }
    }
  }
}
