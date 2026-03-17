// Copyright (C) 2013-2016,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/loader/order/stack.hpp>

#include <gspc/we/loader/macros.hpp>

WE_MOD_INITIALIZE_START()
{
  start().push ("a");
}
WE_MOD_INITIALIZE_END()
