#include <drts/Resource.hpp>

namespace gspc
{
  Resource::Resource (resource::Class resource_class_)
    : Resource (resource_class_, 0)
  {}

  Resource::Resource (resource::Class resource_class_, std::size_t shm_size_)
    : resource_class (resource_class_)
    , shm_size (shm_size_)
  {}

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
