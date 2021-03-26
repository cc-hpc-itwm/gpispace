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

#pragma once

#include <we/type/bytearray.hpp>

#include <boost/filesystem.hpp>

#include <string>

namespace transform_file
{
  class parameter
  {
  public:
    parameter ( boost::filesystem::path const& input
              , boost::filesystem::path const& output
              , unsigned long size
              )
      : _input (input)
      , _output (output)
      , _size (size)
    {}

    boost::filesystem::path const& input() const
    {
      return _input;
    }
    boost::filesystem::path const& output() const
    {
      return _output;
    }
    unsigned long size() const
    {
      return _size;
    }

  private:
    boost::filesystem::path const _input;
    boost::filesystem::path const _output;
    unsigned long _size;
  };

  parameter from_bytearray (we::type::bytearray const&);
  we::type::bytearray to_bytearray (parameter const&);
}
