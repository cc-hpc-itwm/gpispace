// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/expect.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.hpp>
#include <gspc/xml/parse/util/skip.hpp>


  namespace gspc::xml::parse
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
