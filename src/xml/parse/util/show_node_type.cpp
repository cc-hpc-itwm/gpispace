// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/util/show_node_type.hpp>

#include <rapidxml.hpp>

#include <stdexcept>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::string show_node_type (const int t)
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
