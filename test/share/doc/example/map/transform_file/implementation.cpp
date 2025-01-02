// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <map/interface.hpp>
#include <map/transform_file/type.hpp>

#include <fmt/core.h>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

void map_produce ( map::user_data_type const& user_data
                 , map::memory_buffer_type buffer
                 , unsigned long id
                 )
{
  transform_file::parameter const parameter
  {transform_file::from_bytearray (user_data)};
  std::string const input {parameter.input().string()};
  unsigned long const size {parameter.size()};

  std::cout << "read " << input << "[" << id << "]\n";

  std::ifstream stream (input, std::ifstream::binary);

  if (!stream)
  {
    throw std::runtime_error
      {fmt::format ("Could not open '{}' for reading", input)};
  }

  if (!stream.seekg (id * buffer.second, stream.beg))
  {
    throw std::runtime_error
      { fmt::format
          ( "Could not seek to position '{}' of '{}'"
          , input
          , (id * buffer.second)
          )
      };
  }

  if (!stream.read ( static_cast<char*> (buffer.first)
                   , std::min (buffer.second, size - id * buffer.second)
                   )
     )
  {
    throw std::runtime_error {fmt::format ("Could not read from '{}'", input)};
  }
}

void map_process
  ( map::user_data_type const&
  , map::const_memory_buffer_type input
  , map::memory_buffer_type output
  )
{
  std::cout << "transform\n";

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

  char const* const mem_input (static_cast<char const*> (input.first));
  char* const mem_output (static_cast<char*> (output.first));

  std::transform ( mem_input, mem_input + input.second
                 , mem_output
                 , [](unsigned char c) { return std::tolower (c); }
                 );
}

void map_consume ( map::user_data_type const& user_data
                 , map::const_memory_buffer_type buffer
                 , unsigned long id
                 )
{
  transform_file::parameter const parameter
  {transform_file::from_bytearray (user_data)};
  std::string const output {parameter.output().string()};
  unsigned long const size {parameter.size()};

  std::cout << "write " << output << "[" << id << "]\n";

  std::fstream stream
    (output, std::fstream::binary | std::fstream::out | std::fstream::in);

  if (!stream)
  {
    throw std::runtime_error
      {fmt::format ("Could not open '{}' for writing", output)};
  }

  if (!stream.seekp (id * buffer.second, stream.beg))
  {
    throw std::runtime_error
      { fmt::format
          ( "Could not seek to position '{}' of '{}'"
          , id * buffer.second
          , output
          )
      };
  }

  if (!stream.write ( static_cast<char const*> (buffer.first)
                    , std::min (buffer.second, size - id * buffer.second)
                    )
     )
  {
    throw std::runtime_error {fmt::format ("Could not write to '{}'", output)};
  }
}
