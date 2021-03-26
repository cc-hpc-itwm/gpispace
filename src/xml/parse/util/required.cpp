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

#include <xml/parse/util/required.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

namespace xml
{
  namespace parse
  {
    std::string required ( const std::string& pre
                         , const xml_node_type* node
                         , const Ch* attr
                         , const state::type& state
                         )
    {
      if (!node->first_attribute (attr))
      {
        throw error::missing_attr (pre, attr, state.position (node));
      }

      return std::string ( node->first_attribute (attr)->value()
                         , node->first_attribute (attr)->value_size()
                         );
    }
  }
}
