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

// \note Not using c++11 attribute syntax `[[gnu::visibility
// ("default")]]`, because we mark friend functions as exported as
// well, inside the class definition. That is supported with the old
// syntax, but not the new one, where it requires a friend with
// attribute to be a type (although clang disagrees and doesn't even
// allow attributes for types).
// \todo Compiler detection if we ever bother about non gcc-compatible?
#define GSPC_DLLEXPORT_IMPL __attribute__ ((visibility ("default")))
