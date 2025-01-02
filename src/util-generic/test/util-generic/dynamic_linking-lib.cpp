// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/dynamic_linking-lib.hpp>

int dltest_variable = -1;
void dltest_setter (int x)
{
  dltest_variable = x;
}
int dltest_getter()
{
  return dltest_variable;
}
