#pragma once

#include <drts/util-generic_hash_forward_declare.hpp>

#include <cstdint>
#include <functional>
#include <ostream>

namespace gspc
{
  namespace remote_interface
  {
    struct ID
    {
      std::uint64_t id {0};

      ID& operator++();

      template<typename Archive>
        void serialize (Archive&, unsigned int);

      friend bool operator== (ID const& lhs, ID const& rhs);
      friend std::ostream& operator<< (std::ostream&, ID const&);
      friend bool operator< (ID const& lhs, ID const& rhs);

      friend std::hash<ID>;
    };
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::remote_interface::ID);

#include <drts/remote_interface/ID.ipp>
