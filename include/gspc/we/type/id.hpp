// Copyright (C) 2010,2012-2016,2019,2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/hard_integral_typedef.hpp>
#include <cstdint>

namespace gspc::we
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

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::we::place_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::we::port_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::we::token_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::we::transition_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (gspc::we::place_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (gspc::we::port_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (gspc::we::priority_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (gspc::we::token_id_type)
FHG_UTIL_HARD_INTEGRAL_TYPEDEF_SERIALIZATION (gspc::we::transition_id_type)
