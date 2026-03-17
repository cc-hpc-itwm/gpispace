// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/random/impl.hpp>
#include <gspc/testing/random/integral.hpp>

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (bool)
{
  return random<int>{} (1, 0) == 1;
}
