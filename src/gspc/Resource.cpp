#include <gspc/Resource.hpp>

namespace gspc
{
  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs.resource_class) == std::tie (rhs.resource_class);
  }
  std::ostream& operator<< (std::ostream& os, Resource const& r)
  {
    return os << "resource " << r.resource_class;
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::Resource
  , r
  , r.resource_class
  );
