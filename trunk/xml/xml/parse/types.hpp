// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPES_HPP
#define _XML_PARSE_TYPES_HPP

typedef char Ch;
typedef rapidxml::xml_node<Ch> xml_node_type;
typedef rapidxml::xml_document<Ch> xml_document_type;
typedef rapidxml::file<Ch> input_type;

#include <iostream>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef boost::unordered_map<std::string, std::string> type_map_type;
      typedef boost::unordered_set<std::string> type_get_type;

      // forward declarations for mutual recursive types
      struct function_type;
      struct transition_type;
      struct net_type;

      template<typename Net, typename Trans>
      class transition_get_function;
    }
  }
}

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/require.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/net.hpp>

#endif
