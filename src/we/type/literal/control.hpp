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

#include <gspc/detail/dllexport.hpp>

#include <iosfwd>

namespace we
{
  namespace type
  {
    namespace literal
    {
      struct GSPC_DLLEXPORT control
      {
        GSPC_DLLEXPORT
          friend std::ostream& operator<< (std::ostream&, control const&);
        GSPC_DLLEXPORT
          friend bool operator== (control const&, control const&);

        GSPC_DLLEXPORT
          friend std::size_t hash_value (control const&);
        GSPC_DLLEXPORT
          friend bool operator< (control const&, control const&);

        template<typename Archive> void serialize (Archive&, unsigned int) {}
      };
    }
  }
}

//! \todo REMOVE! This is deprecated but some clients still use it.
typedef we::type::literal::control control;
