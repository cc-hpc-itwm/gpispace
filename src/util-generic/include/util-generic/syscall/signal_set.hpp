// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <initializer_list>

#include <signal.h>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      struct signal_set
      {
        signal_set();
        signal_set (std::initializer_list<int> /* signals */);
        void add (int signal);

        sigset_t _;
      };
    }
  }
}
