// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/syscall/signal_set.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      struct signal_fd
      {
        signal_fd (signal_set const&);
        ~signal_fd();

        signal_fd (signal_fd const&) = delete;
        signal_fd (signal_fd&&) = delete;
        signal_fd& operator= (signal_fd const&) = delete;
        signal_fd& operator= (signal_fd&&) = delete;

        int _;
      };
    }
  }
}
