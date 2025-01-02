// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/util/show_node_type.hpp>

#include <rapidxml.hpp>

#include <stdexcept>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::string show_node_type (int t)
      {
        switch (t)
          {
          case rapidxml::node_document: return "document";
          case rapidxml::node_element: return "element";
          case rapidxml::node_data: return "data";
          case rapidxml::node_cdata: return "cdata";
          case rapidxml::node_comment: return "comment";
          case rapidxml::node_declaration: return "declaration";
          case rapidxml::node_doctype: return "doctype";
          case rapidxml::node_pi: return "pi";
          default: throw std::runtime_error ("STRANGE: unknown node type");
          }
      }
    }
  }
}
