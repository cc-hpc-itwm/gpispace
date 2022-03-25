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

#include <util-generic/hard_integral_typedef.hpp>

namespace we
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 > 194 years.
  // It follows that an uint64_t is enough for now.

  FHG_UTIL_HARD_INTEGRAL_TYPEDEF (place_id_type, std::uint64_t);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF (port_id_type, std::uint64_t);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF (priority_type, std::uint16_t);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF (token_id_type, std::uint64_t);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF (transition_id_type, std::uint64_t);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (place_id_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (port_id_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (priority_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ISTREAM_OPERATOR (token_id_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR (place_id_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR (port_id_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR (priority_type);
  FHG_UTIL_HARD_INTEGRAL_TYPEDEF_OSTREAM_OPERATOR (token_id_type);
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (we::place_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (we::port_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (we::token_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (we::transition_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (we::place_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (we::port_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (we::priority_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (we::token_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (we::transition_id_type)
