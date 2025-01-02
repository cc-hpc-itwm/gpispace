// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random/impl.hpp>
#include <util-generic/testing/random/integral.hpp>

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (bool)
{
  return random<int>{} (1, 0) == 1;
}
