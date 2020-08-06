#pragma once

#include <drts/remote_interface/ID.hpp>
#include <drts/util-generic_hash_forward_declare.hpp>

#include <cstdint>
#include <ostream>

namespace gspc
{
  namespace resource
  {
    struct ID
    {
      ID (remote_interface::ID, std::uint64_t);
      ID (remote_interface::ID);

      remote_interface::ID remote_interface;
      std::uint64_t id;

      ID& operator++();

      //! \note for serialization
      ID() = default;
      template<typename Archive>
        void serialize (Archive&, unsigned int);

      friend bool operator== (ID const& lhs, ID const& rhs);
      friend std::ostream& operator<< (std::ostream&, ID const&);
    };
  }
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::resource::ID);

#include <drts/resource/ID.ipp>
