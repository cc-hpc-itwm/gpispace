// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/syscall/signal_fd.hpp>

#include <util-generic/syscall.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      signal_fd::signal_fd (signal_set const& set)
        : _ (syscall::signalfd (-1, &set._, 0))
      {}
      signal_fd::~signal_fd()
      {
        syscall::close (_);
      }
    }
  }
}
