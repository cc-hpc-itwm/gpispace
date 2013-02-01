// mirko.rahn@itwm.fraunhofer.de

#include <we/type/control.hpp>

#include <string>

namespace we
{
  namespace type
  {
    bool operator== (const control&, const control&)
    {
      return true;
    }

    std::ostream& operator<< (std::ostream& os, const control&)
    {
      return os << std::string("[]");
    }
  }
}
