// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/loader/test/order/stack.hpp>

#include <we/loader/macros.hpp>

WE_MOD_INITIALIZE_START()
{
  start().push ("a");
}
WE_MOD_INITIALIZE_END()
