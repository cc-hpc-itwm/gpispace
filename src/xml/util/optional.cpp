// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/optional.hpp>
#include <optional>


  namespace gspc::xml::parse
  {
    std::optional<std::string> optional ( const xml_node_type* node
                                          , const Ch* attr
                                          )
    {
      if (node->first_attribute (attr))
      {
        return std::string ( node->first_attribute (attr)->value()
                           , node->first_attribute (attr)->value_size()
                           );
      }
      return {};
    }
  }
