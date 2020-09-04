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

#include <xml/parse/util/expect.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type t
                        , const state::type& state
                        )
    {
      skip (node, rapidxml::node_comment);

      if (node && node->type() != t)
      {
        throw error::wrong_node (t, node->type(), state.position (node));
      }
    }
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type t1
                        , const rapidxml::node_type t2
                        , const state::type& state
                        )
    {
      skip (node, rapidxml::node_comment);

      if (node && node->type() != t1 && node->type() != t2)
      {
        throw error::wrong_node (t1, t2, node->type(), state.position (node));
      }
    }
  }
}
