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
      struct process_signal_block
      {
        process_signal_block (signal_set const&);
        ~process_signal_block();

        process_signal_block (process_signal_block const&) = delete;
        process_signal_block (process_signal_block&&) = delete;
        process_signal_block& operator= (process_signal_block const&) = delete;
        process_signal_block& operator= (process_signal_block&&) = delete;

      private:
        signal_set _old_set;
      };
    }
  }
}
