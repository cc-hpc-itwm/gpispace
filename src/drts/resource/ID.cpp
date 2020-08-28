#include <drts/resource/ID.hpp>

#include <tuple>

namespace gspc
{
  namespace resource
  {
    ID::ID (remote_interface::ID rif, std::uint64_t arg_id)
      : remote_interface (rif)
      , id (arg_id)
    {}

    ID::ID (remote_interface::ID rif)
      : ID (rif, 0)
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

    ID::operator std::string() const
    {
      std::ostringstream osstr;
      osstr << *this;

      return osstr.str();
    }
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::resource::ID
  , x
  , x.id
  , x.remote_interface
  );
