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

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace segment
      {
        enum segment_type
          {
            // maximum of 4 bits available --> see handle_t
            SEG_INVAL    = 0
          , SEG_GASPI    = 1    // GASPI based segment
          , SEG_SHM      = 2    // SHM based segment
          , SEG_BEEGFS   = 3    // (parallel) beegfs shared file-system based segment
          , SEG_MAX_TYPE = ((1 << 4) - 1)
          };
      }
    }
  }
}
