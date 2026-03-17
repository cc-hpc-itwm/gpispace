// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/cdata.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.hpp>
#include <gspc/xml/parse/util/expect.hpp>


  namespace gspc::xml::parse
  {
    std::list<std::string> parse_cdata ( const xml_node_type* node
                                       , state::type const& state
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
