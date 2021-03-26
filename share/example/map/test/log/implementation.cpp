// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <map/interface.hpp>

#include <boost/format.hpp>

#include <iostream>
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
    (static_cast<unsigned long const*> (input.first));
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
    (static_cast<unsigned long const*> (buffer.first));

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
