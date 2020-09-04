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

#include <gpi-space/gpi/system.hpp>

#include <util-generic/syscall.hpp>

namespace sys
{
  uint64_t get_total_memory_size ()
  {
    const long total_pages = fhg::util::syscall::sysconf (_SC_PHYS_PAGES);
    return total_pages * fhg::util::syscall::sysconf (_SC_PAGESIZE);
  }

  uint64_t get_avail_memory_size ()
  {
    const long avail_pages = fhg::util::syscall::sysconf (_SC_AVPHYS_PAGES);
    return avail_pages * fhg::util::syscall::sysconf (_SC_PAGESIZE);
  }
}
