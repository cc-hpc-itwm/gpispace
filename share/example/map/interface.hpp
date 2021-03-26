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
                   );

  void map_process ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , map::memory_buffer_type
                   );

  void map_consume ( map::user_data_type const&
                   , map::const_memory_buffer_type
                   , unsigned long id
                   );
}
