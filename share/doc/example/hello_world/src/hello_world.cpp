// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <hello_world.hpp>
#include <iostream>
#include <unistd.h>

void impl_hello_world (void)
{
  std::cout << "*** (CPP) Hello " << std::flush;

  sleep (1);

  std::cout << "World *** (CPP)" << std::endl;
}
