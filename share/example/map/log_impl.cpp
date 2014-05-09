#include <map_interface.hpp>

#include <fhglog/LogMacros.hpp>

namespace map
{
  // typedef bytearray::type user_data_type;
  // typedef unsigned long size_in_bytes_type;
  // typedef std::pair<void*, size_in_bytes_type> memory_buffer_type;
  // typedef std::pair<void const*, size_in_bytes_type> const_memory_buffer_type;

  void produce ( user_data_type const& user_data
               , memory_buffer_type buffer
               , unsigned long id
               )
  {
    LOG ( INFO, "produce"
        << ": user_data = " << user_data
        << ", buffer_size = " << buffer.second
        << ", id = " << id
        );
  }

  void process
    ( user_data_type const& user_data
    , const_memory_buffer_type input
    , memory_buffer_type output
    )
  {
    LOG ( INFO, "process"
        << ": user_data = " << user_data
        << ", input_buffer_size = " << input.second
        << ", output_buffer_size = " << output.second
        );
  }

  void consume ( user_data_type const& user_data
               , const_memory_buffer_type buffer
               , unsigned long id
               )
  {
    LOG ( INFO, "consume"
        << ": user_data = " << user_data
        << ", buffer_size = " << buffer.second
        << ", id = " << id
        );
  }
}
