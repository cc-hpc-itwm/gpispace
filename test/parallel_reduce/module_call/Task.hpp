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

#pragma once

#include <iosfwd>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        struct Task
        {
          unsigned long _lhs;
          unsigned long _rhs;

          Task (decltype (_lhs), decltype (_rhs));

          template<typename Archive> void serialize (Archive&, unsigned int);
          Task() = default;
        };

        std::ostream& operator<< (std::ostream&, Task const&);
        bool operator== (Task const&, Task const&);
      }
    }
  }
}

#include <test/parallel_reduce/module_call/Task.ipp>
