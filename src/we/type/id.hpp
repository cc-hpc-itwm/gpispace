// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/hard_integral_typedef.hpp>
#include <cstdint>

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
