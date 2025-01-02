// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/expect.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void expect_none_or ( xml_node_type*& node
                        , rapidxml::node_type t
                        , state::type const& state
                        )
    {
      skip (node, rapidxml::node_comment);

      if (node && node->type() != t)
      {
        throw error::wrong_node (t, node->type(), state.position (node));
      }
    }
    void expect_none_or ( xml_node_type*& node
                        , rapidxml::node_type t1
                        , rapidxml::node_type t2
                        , state::type const& state
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
