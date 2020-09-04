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

#pragma once

#include <inttypes.h>
#include <limits.h>
#include <string>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      typedef uint64_t size_t;
      typedef uint64_t offset_t;
      typedef uint64_t id_t;
      typedef uint64_t ref_count_t;
      typedef ::time_t time_t;
      typedef id_t segment_id_t;
      typedef id_t handle_id_t;
      typedef id_t queue_id_t;
      typedef id_t process_id_t;
      typedef id_t rank_t;

      using memcpy_id_t = id_t;

      typedef uint32_t error_t;
      typedef uint16_t mode_t;
      typedef uint16_t flags_t;
      typedef std::string path_t;
      typedef std::string name_t;

#define GPI_PC_INVAL (::gpi::pc::type::id_t)(-1)
    }
  }

  namespace flag
  {
    inline
    bool is_set (const pc::type::flags_t f, const pc::type::flags_t mask)
    {
      return (f & mask);
    }

    inline
    void set (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f |= mask;
    }

    inline
    void clear (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f &= ~mask;
    }

    inline
    void unset (pc::type::flags_t & f, const pc::type::flags_t mask)
    {
      f &= ~mask;
    }
  }
}
