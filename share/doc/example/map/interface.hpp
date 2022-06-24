// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <gspc/detail/dllexport.hpp>

#include <we/type/bytearray.hpp>

#include <utility>

namespace map
{
  using user_data_type = we::type::bytearray;
  using size_in_bytes_type = unsigned long;
  using memory_buffer_type = std::pair<void*, size_in_bytes_type>;
  using const_memory_buffer_type = std::pair<const void*, size_in_bytes_type>;
}

extern "C"
{
  GSPC_DLLEXPORT void map_produce ( map::user_data_type const&
                                  , map::memory_buffer_type
                                  , unsigned long id
                                  );

  GSPC_DLLEXPORT void map_process ( map::user_data_type const&
                                  , map::const_memory_buffer_type
                                  , map::memory_buffer_type
                                  );

  GSPC_DLLEXPORT void map_consume ( map::user_data_type const&
                                  , map::const_memory_buffer_type
                                  , unsigned long id
                                  );
}
