// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
