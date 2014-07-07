#include <interface.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/format.hpp>

#include <stdexcept>

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

    unsigned long* const mem (static_cast<unsigned long*> (buffer.first));

    std::fill (mem, mem + buffer.second / sizeof (unsigned long), id);
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

    if (input.second != output.second)
    {
      throw std::logic_error
        ( ( boost::format ("input/output buffer sizes differ: %1% != %2%")
          % input.second
          % output.second
          ).str()
        );
    }

    unsigned long const* const mem_input
      (static_cast<unsigned long const* const> (input.first));
    unsigned long* const mem_output
      (static_cast<unsigned long*> (output.first));

    for (unsigned long i (0); i < input.second / sizeof (unsigned long); ++i)
    {
      mem_output[i] = std::numeric_limits<unsigned long>::max() - mem_input[i];
    }
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

    unsigned long const* const mem
      (static_cast<unsigned long const* const> (buffer.first));

    for (unsigned long i (0); i < buffer.second / sizeof (unsigned long); ++i)
    {
      if (mem[i] != std::numeric_limits<unsigned long>::max() - id)
      {
        throw std::logic_error
          ( ( boost::format ("verify failed: [id = %1%, i = %2%]: %2% != %4%")
            % id
            % i
            % mem[i]
            % (std::numeric_limits<unsigned long>::max() - id)
            ).str()
          );
      }
    }
  }
}
