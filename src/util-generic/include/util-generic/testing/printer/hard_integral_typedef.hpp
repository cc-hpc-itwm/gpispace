// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/hard_integral_typedef.hpp>

#define FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER(hit_)          \
  FHG_BOOST_TEST_LOG_VALUE_PRINTER (hit_, os, val)                      \
  {                                                                     \
    os << to_string (val);                                              \
  }
