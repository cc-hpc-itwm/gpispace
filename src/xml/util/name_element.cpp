// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/name_element.hpp>

#include <gspc/xml/parse/state.hpp>
#include <gspc/xml/parse/util/expect.hpp>


  namespace gspc::xml::parse
  {
    std::string name_element ( xml_node_type*& node
                             , state::type const& state
                             )
    {
      expect_none_or (node, rapidxml::node_element, state);

      if (!node)
      {
        return "<missing_node>";
      }

      return std::string (node->name(), node->name_size());
    }
  }
