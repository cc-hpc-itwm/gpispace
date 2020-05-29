#include <we/type/literal/control.hpp>

#include <iostream>
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

      bool operator< (const control&, const control&)
      {
        return false;
      }
    }
  }
}
