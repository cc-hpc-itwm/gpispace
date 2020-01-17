#pragma once

#include <gspc/util-generic_hash_forward_declare.hpp>

#include <cstdint>
#include <ostream>

namespace gspc
{
  namespace task
  {
    struct ID
    {
      //! \todo  more complex:
      //! - hierarchy with user_context
      //! - information for scheduler
      std::uint64_t id;

      ID& operator++();

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);

      friend std::ostream& operator<< (std::ostream&, ID const&);
      friend bool operator== (ID const&, ID const&);
    };
  }
}
UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::task::ID);

#include <gspc/task/ID.ipp>
