// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/type/value.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class GSPC_DLLEXPORT show
      {
      public:
        show (value_type const&);
        std::ostream& operator() (std::ostream&) const;
      private:
        value_type const& _value;
      };
      GSPC_DLLEXPORT std::ostream& operator<< (std::ostream&, show const&);
    }
  }
}
