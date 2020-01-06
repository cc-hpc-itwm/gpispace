#include <gspc/Resource.hpp>

#include <iostream>

namespace gspc
{
  namespace resource
  {
    std::ostream& operator<< (std::ostream& os, ID const& id)
    {
      return os << "resource " << to_string (id);
    }
  }
}
