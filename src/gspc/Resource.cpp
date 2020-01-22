#include <gspc/Resource.hpp>

#include <boost/optional/optional_io.hpp>

namespace gspc
{
  Resource::Resource (resource::Class resource_class_)
    : resource_class (resource_class_)
  {}

  Resource::Resource (resource::Class resource_class_, resource::Class proxy_)
    : resource_class (resource_class_)
    , proxy (proxy_)
  {}

  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs.resource_class, lhs.proxy)
      == std::tie (rhs.resource_class, rhs.proxy)
      ;
  }
  std::ostream& operator<< (std::ostream& os, Resource const& r)
  {
    os << "resource " << r.resource_class;

    if (r.proxy)
    {
      os << " (proxy " << r.proxy << ")";
    }

    return os;
  }
}

//! \todo move to util-generic? already there somewhere?
namespace std
{
  template <typename T>
    struct hash<boost::optional<T>>
  {
    std::size_t operator() (boost::optional<T> const& arg) const
    {
      return arg ? std::hash<T>{}(*arg) : 0;
    }
  };
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::Resource
  , r
  , r.resource_class
  , r.proxy
  );
