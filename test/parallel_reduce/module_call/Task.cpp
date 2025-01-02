// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/parallel_reduce/module_call/Task.hpp>

#include <iostream>
#include <tuple>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        Task::Task (decltype (_lhs) lhs, decltype (_rhs) rhs)
          : _lhs (lhs)
          , _rhs (rhs)
        {}

        std::ostream& operator<< (std::ostream& os, Task const& task)
        {
          return os << "Task {" << task._lhs << ", " << task._rhs << "}";
        }
        bool operator== (Task const& lhs, Task const& rhs)
        {
        #define ESSENCE(x) std::tie (x._lhs, x._rhs)
          return ESSENCE (lhs) == ESSENCE (rhs);
        #undef ESSENCE
        }
      }
    }
  }
}
