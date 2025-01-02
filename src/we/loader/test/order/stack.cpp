// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/loader/test/order/stack.hpp>

std::stack<std::string>& start()
{
  static std::stack<std::string> s;

  return s;
}
