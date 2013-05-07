// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/show_node_type.hpp>

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

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
