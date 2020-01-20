#pragma once

#include <gspc/task/ID.hpp>

#include <gspc/util-generic_hash_forward_declare.hpp>

#include <cstdint>

namespace gspc
{
  namespace job
  {
    struct ID
    {
      //! \todo  more complex:
      //! - hierarchy with user_context
      //! - information for scheduler
      std::uint64_t id;
      //! \todo multiple tasks per job!?
      task::ID task_id;

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);

      friend std::ostream& operator<< (std::ostream&, ID const&);
      friend bool operator== (ID const&, ID const&);
    };
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::job::ID);

#include <gspc/job/ID.ipp>
