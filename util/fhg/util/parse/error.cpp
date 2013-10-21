// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/error.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace error
      {
        expected::expected (const std::string& what, const position& inp)
          : generic (boost::format ("expected '%1%'") % what, inp)
        {}
      }
    }
  }
}
