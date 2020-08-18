#include <drts/remote_interface/ID.hpp>

#include <tuple>

namespace gspc
{
  namespace remote_interface
  {
    ID& ID::operator++()
    {
      ++id;
      return *this;
    }

    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.id) == std::tie (rhs.id);
    }
    std::ostream& operator<< (std::ostream& os, ID const& x)
    {
      return os << x.id;
    }
    bool operator< (ID const& lhs, ID const& rhs)
    {
      return lhs.id < rhs.id;
    }

    bool operator== (ID const& lhs, ID const& rhs);
    std::ostream& operator<< (std::ostream&, ID const&);
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::remote_interface::ID
  , x
  , x.id
  );
