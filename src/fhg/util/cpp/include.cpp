// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/cpp/include.hpp>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      include::include (const std::string& fname)
        : _fname (fname)
      {}
      std::ostream& include::operator() (std::ostream& os) const
      {
        return os << "#include <" << _fname << ">" << std::endl;
      }
    }
  }
}