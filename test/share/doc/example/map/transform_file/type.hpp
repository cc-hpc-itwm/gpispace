// Copyright (C) 2014,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/bytearray.hpp>

#include <filesystem>

#include <string>

namespace transform_file
{
  class parameter
  {
  public:
    parameter ( std::filesystem::path const& input
              , std::filesystem::path const& output
              , unsigned long size
              )
      : _input (input)
      , _output (output)
      , _size (size)
    {}

    std::filesystem::path input() const
    {
      return _input;
    }
    std::filesystem::path output() const
    {
      return _output;
    }
    unsigned long size() const
    {
      return _size;
    }

  private:
    std::filesystem::path _input;
    std::filesystem::path _output;
    unsigned long _size;
  };

  parameter from_bytearray (gspc::we::type::bytearray const&);
  gspc::we::type::bytearray to_bytearray (parameter const&);
}
