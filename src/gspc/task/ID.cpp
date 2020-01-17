#include <gspc/task/ID.hpp>

namespace gspc
{
  namespace task
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
      return os << "task " << x.id;
    }
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::task::ID
  , x
  , x.id
  );
