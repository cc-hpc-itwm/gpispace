// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
