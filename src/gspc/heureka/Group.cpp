#include <gspc/heureka/Group.hpp>

namespace gspc
{
  namespace heureka
  {
    Group& Group::operator++()
    {
      ++id;
      return *this;
    }
    bool operator== (Group const& lhs, Group const& rhs)
    {
      return std::tie (lhs.id) == std::tie (rhs.id);
    }
    std::ostream& operator<< (std::ostream& os, Group const& x)
    {
      return os << "heureka_group " << x.id;
    }
    std::size_t hash_value (Group const& g)
    {
      return std::hash<Group>{} (g);
    }
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::heureka::Group
  , x
  , x.id
  );
