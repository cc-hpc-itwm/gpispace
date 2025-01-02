// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/syscall/process_signal_block.hpp>

#include <util-generic/syscall.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      process_signal_block::process_signal_block (signal_set const& set)
      {
        syscall::sigprocmask (SIG_BLOCK, &set._, &_old_set._);
      }
      process_signal_block::~process_signal_block()
      {
        syscall::sigprocmask (SIG_SETMASK, &_old_set._, nullptr);
      }
    }
  }
}
