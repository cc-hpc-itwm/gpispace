// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/required.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

namespace xml
{
  namespace parse
  {
    std::string required ( std::string const& pre
                         , const xml_node_type* node
                         , const Ch* attr
                         , state::type const& state
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
