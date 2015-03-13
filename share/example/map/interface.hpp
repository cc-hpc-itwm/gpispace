#pragma once

#include <we/type/bytearray.hpp>

#include <drts/worker/context.hpp>

#include <utility>

namespace map
{
  typedef we::type::bytearray user_data_type;
  typedef unsigned long size_in_bytes_type;
  typedef std::pair<void*, size_in_bytes_type> memory_buffer_type;
  typedef std::pair<void const*, size_in_bytes_type> const_memory_buffer_type;
}

extern "C"
{
  void map_produce ( map::user_data_type const&
                   , map::memory_buffer_type
                   , unsigned long id
                   , drts::worker::logger_type
                   );

  void map_process ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , map::memory_buffer_type
                   , drts::worker::logger_type
                   );

  void map_consume ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , unsigned long id
                   , drts::worker::logger_type
                   );
}

#define MAP_LOG(_message) GSPC_LLOG (INFO, _message, logger)
