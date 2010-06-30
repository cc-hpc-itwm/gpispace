// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPES_HPP
#define _XML_PARSE_TYPES_HPP

typedef char Ch;
typedef rapidxml::xml_node<Ch> xml_node_type;
typedef rapidxml::xml_document<Ch> xml_document_type;
typedef rapidxml::file<Ch> input_type;

#include <parse/type/connect.hpp>
#include <parse/type/expression.hpp>
#include <parse/type/mod.hpp>
#include <parse/type/place.hpp>
#include <parse/type/port.hpp>
#include <parse/type/token.hpp>

#endif
