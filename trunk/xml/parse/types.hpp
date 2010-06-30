// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPES_HPP
#define _XML_PARSE_TYPES_HPP

typedef char Ch;
typedef rapidxml::xml_node<Ch> xml_node_type;
typedef rapidxml::xml_document<Ch> xml_document_type;
typedef rapidxml::file<Ch> input_type;

#include <boost/variant.hpp>
#include <iostream>

// forward declarations for mutual recursive types
namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct function;
      struct transition;
      struct net;

      // generic show visitor
      namespace visitor
      {
        class show : public boost::static_visitor<std::ostream &>
        {
        private:
          std::ostream & s;

        public:
          show (std::ostream & _s) : s (_s) {}

          template<typename T>
          std::ostream & operator () (const T & x) const
          {
            return s << x;
          }
        };
      }

      struct level { int l; level (int _l) : l(_l) {} };

      std::ostream & operator << (std::ostream & s, const level & l)
      {
        for (int i (0); i < l.l; ++i)
          {
            s << "  ";
          }
        return s;
      }
    }
  }
}

#include <parse/type/connect.hpp>
#include <parse/type/expression.hpp>
#include <parse/type/mod.hpp>
#include <parse/type/place.hpp>
#include <parse/type/port.hpp>
#include <parse/type/token.hpp>

#include <parse/type/function.hpp>
#include <parse/type/transition.hpp>
#include <parse/type/net.hpp>

#endif
