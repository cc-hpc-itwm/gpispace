// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <map/interface.hpp>

#include <fmt/core.h>
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

  auto* const mem (static_cast<unsigned long*> (buffer.first));

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
      { fmt::format
          ( "input/output buffer sizes differ: {} != {}"
          , input.second
          , output.second
          )
      };
  }

  auto const* const mem_input
    (static_cast<unsigned long const*> (input.first));
  auto* const mem_output
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

  auto const* const mem
    (static_cast<unsigned long const*> (buffer.first));

  for (unsigned long i (0); i < buffer.second / sizeof (unsigned long); ++i)
  {
    if (mem[i] != std::numeric_limits<unsigned long>::max() - id)
    {
      throw std::logic_error
        { fmt::format
            ( "verify failed: [id = {}, i = {}]: {} != {}"
            , id
            , i
            , mem[i]
            , std::numeric_limits<unsigned long>::max() - id
            )
        };
    }
  }
}
