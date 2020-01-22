#include <gspc/job/ID.hpp>

namespace gspc
{
  namespace job
  {
    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.id, lhs.task_id) == std::tie (rhs.id, rhs.task_id);
    }
    std::ostream& operator<< (std::ostream& os, ID const& x)
    {
      return os << "job " << x.id << " " << x.task_id;
    }
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DEFINE
  ( gspc::job::ID
  , x
  , x.id
  , x.task_id
  );
