#pragma once

#include <gspc/util-generic_hash_forward_declare.hpp>

#include <cstdint>

namespace gspc
{
  namespace heureka
  {
    struct Group
    {
      std::uint64_t id;

      Group& operator++();

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);

      friend std::ostream& operator<< (std::ostream&, Group const&);
      friend bool operator== (Group const&, Group const&);
    };

    std::size_t hash_value (Group const&);
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::heureka::Group);

#include <gspc/heureka/Group.ipp>
