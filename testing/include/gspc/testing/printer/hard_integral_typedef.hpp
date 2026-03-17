// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>
#include <gspc/util/hard_integral_typedef.hpp>

#define GSPC_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER(hit_)              \
  GSPC_BOOST_TEST_LOG_VALUE_PRINTER (hit_, os, val)                     \
  {                                                                     \
    os << to_string (val);                                              \
  }
