// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RPC_COMMON_HPP
#define FHG_RPC_COMMON_HPP

#include <cstdint>

namespace fhg
{
  namespace rpc
  {
    struct packet_header
    {
      uint64_t message_id;
      std::size_t buffer_size;
      char buffer[0];

      packet_header (uint64_t id, std::size_t size)
        : message_id (id)
        , buffer_size (size)
      {}
    };
  }
}

#endif
