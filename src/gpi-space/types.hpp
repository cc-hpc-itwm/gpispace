// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <inttypes.h>

#include <boost/dynamic_bitset.hpp>

namespace gpi
{
  typedef uint64_t offset_t;
  using rank_t = unsigned short;
  using queue_desc_t = unsigned char;
  typedef uint64_t size_t;
  typedef float    version_t;
  typedef unsigned short port_t;
  typedef boost::dynamic_bitset<> error_vector_t;

  using notification_t = unsigned int;
  using notification_id_t = unsigned short;

  using timeout_t = unsigned long;
  using segment_id_t = unsigned char;

  using netdev_id_t = int;
}
