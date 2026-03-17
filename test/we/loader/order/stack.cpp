// Copyright (C) 2013-2015,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/loader/order/stack.hpp>

std::stack<std::string>& start()
{
  static std::stack<std::string> s;

  return s;
}
