// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_SHOW_NODE_TYPE_HPP
#define _XML_PARSE_UTIL_SHOW_NODE_TYPE_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      inline std::string
      show_node_type (const int t)
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

      inline std::string
      quote (const std::string & str)
      {
        return "\"" + str + "\"";
      }
    }
  }
}

#endif
