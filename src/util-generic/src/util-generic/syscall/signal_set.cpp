// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/syscall/signal_set.hpp>

#include <util-generic/syscall.hpp>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
      signal_set::signal_set()
      {
        syscall::sigemptyset (&_);
      }

      signal_set::signal_set (std::initializer_list<int> signals)
        : signal_set()
      {
        for (int const& signal : signals)
        {
          add (signal);
        }
      }

      void signal_set::add (int signal)
      {
        syscall::sigaddset (&_, signal);
      }
    }
  }
}
