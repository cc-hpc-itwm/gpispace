// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/required.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.hpp>


  namespace gspc::xml::parse
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
