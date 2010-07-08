// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <string>
#include <iostream>

#include <parse/util/join.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::vector<std::string> expression_vec_type;

      struct expression_type
      {
        expression_vec_type expressions;

        expression_type () : expressions () {}

        expression_type (const expression_vec_type & _expressions)
          : expressions (_expressions)
        {}

        std::string expression (void) const
        {
          return util::join (expressions.begin(), expressions.end(), "; ");
        }
      };

      std::ostream & operator << (std::ostream & s, const expression_type & e)
      {
        return s << "expression (" << e.expression() << ") // expression";
      }
    }
  }
}

#endif
