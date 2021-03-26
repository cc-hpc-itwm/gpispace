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

#include <fhg/util/cpp/include_guard.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace include_guard
      {
        open::open (const std::string& name)
          : _name (name)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          return os << "#ifndef _" << _name << std::endl
                    << "#define _" << _name << std::endl
                    << std::endl;
        }

        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << "#endif" << std::endl;
        }
      }
    }
  }
}
