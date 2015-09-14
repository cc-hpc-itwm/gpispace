#include <map/interface.hpp>

#include <boost/format.hpp>

#include <stdexcept>

void map_produce ( map::user_data_type const& user_data
                 , map::memory_buffer_type buffer
                 , unsigned long id
                 )
{
  std::cout << "produce"
            << ": user_data = " << user_data
            << ", buffer_size = " << buffer.second
            << ", id = " << id
            << '\n';

  unsigned long* const mem (static_cast<unsigned long*> (buffer.first));

  std::fill (mem, mem + buffer.second / sizeof (unsigned long), id);
}

void map_process
  ( map::user_data_type const& user_data
  , map::const_memory_buffer_type input
  , map::memory_buffer_type output
  )
{
  std::cout << "process"
            << ": user_data = " << user_data
            << ", input_buffer_size = " << input.second
            << ", output_buffer_size = " << output.second
            << '\n';

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

void map_consume ( map::user_data_type const& user_data
                 , map::const_memory_buffer_type buffer
                 , unsigned long id
                 )
{
  std::cout << "consume"
            << ": user_data = " << user_data
            << ", buffer_size = " << buffer.second
            << ", id = " << id
            << '\n';

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
