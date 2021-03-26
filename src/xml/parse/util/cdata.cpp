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

#include <xml/parse/util/cdata.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util/expect.hpp>

namespace xml
{
  namespace parse
  {
    std::list<std::string> parse_cdata ( const xml_node_type* node
                                       , const state::type& state
                                       )
    {
      std::list<std::string> v;

      for ( xml_node_type* child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
      {
        expect_none_or (child, rapidxml::node_data, rapidxml::node_cdata, state);

        if (child)
        {
          v.push_back (std::string (child->value(), child->value_size()));
        }
      }

      return v;
    }
  }
}
