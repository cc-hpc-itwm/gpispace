// Copyright (C) 2013,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/skip.hpp>


  namespace gspc::xml::parse
  {
    void skip (xml_node_type*& node, rapidxml::node_type t)
    {
      while (node && (node->type() == t))
        {
          node = node->next_sibling();
        }
    }
  }
