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

#include <xml/parse/type/with_position_of_definition.hpp>

namespace xml
{
  namespace parse
  {
    with_position_of_definition::with_position_of_definition
      (util::position_type const& position_of_definition)
        : _position_of_definition (position_of_definition)
    {}

    util::position_type const&
    with_position_of_definition::position_of_definition() const
    {
      return _position_of_definition;
    }
  }
}
