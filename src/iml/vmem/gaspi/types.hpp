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

#include <cstdint>

namespace gpi
{
  using offset_t = std::uint64_t;
  using rank_t = unsigned short;
  using queue_desc_t = unsigned char;
  using size_t = std::uint64_t;
  using port_t = unsigned short;

  using notification_t = unsigned int;
  using notification_id_t = unsigned short;

  using timeout_t = unsigned long;
  using segment_id_t = unsigned char;
}
