// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void skip (xml_node_type*& node, rapidxml::node_type t)
    {
      while (node && (node->type() == t))
        {
          node = node->next_sibling();
        }
    }
  }
}
