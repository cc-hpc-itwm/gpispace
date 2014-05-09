#pragma once

#include <we/type/bytearray.hpp>

#include <utility>

namespace map
{
  typedef bytearray::type user_data_type;
  typedef unsigned long size_in_bytes_type;
  typedef std::pair<void*, size_in_bytes_type> memory_buffer_type;
  typedef std::pair<void const*, size_in_bytes_type> const_memory_buffer_type;

  void produce
    (user_data_type const&, memory_buffer_type, unsigned long id);

  void process
    (user_data_type const&, const_memory_buffer_type, memory_buffer_type);

  void consume
    (user_data_type const&, const_memory_buffer_type, unsigned long id);
}
