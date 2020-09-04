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

#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

void do_write ( const std::string & filename
              , const std::string & type
              , const long & part      // ordinal number of this part
              , const long & part_size // size of a full part, bytes
              , const long & size      // size of this part, bytes
              , const long & num       // number of traces in this part
              , void * pos
              );

#ifdef __cplusplus
}
#endif
