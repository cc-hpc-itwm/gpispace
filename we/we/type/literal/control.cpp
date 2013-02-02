// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/control.hpp>

#include <string>

namespace we
{
  namespace type
  {
    namespace literal
    {
      bool operator== (const control&, const control&)
      {
        return true;
      }

      std::ostream& operator<< (std::ostream& os, const control&)
      {
        return os << std::string("[]");
      }

      std::size_t hash_value (const control&)
      {
        return 42;
      }
    }
  }
}
