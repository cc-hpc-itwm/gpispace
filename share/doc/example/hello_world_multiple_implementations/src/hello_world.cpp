// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <hello_world.hpp>

std::string impl_hello_world_cpu (void)
{
  return "cpu";
}

std::string impl_hello_world_gpu (void)
{
  return "gpu";
}
