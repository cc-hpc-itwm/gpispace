#include <drts/resource/ID.hpp>

#include <tuple>

namespace gspc
{
  namespace resource
  {
    ID::ID (remote_interface::ID rif)
      : remote_interface (rif)
      , id (0)
    {}

    ID& ID::operator++()
    {
      ++id;
      return *this;
    }

    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.remote_interface, lhs.id)
        == std::tie (rhs.remote_interface, rhs.id);
    }
    std::ostream& operator<< (std::ostream& os, ID const& x)
    {
      return os << x.remote_interface << " resource " << x.id;
    }
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::resource::ID
  , x
  , x.id
  , x.remote_interface
  );
