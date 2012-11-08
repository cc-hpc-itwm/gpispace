// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <xml/parse/type/function.fwd.hpp>

#include <xml/parse/id/generic.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> expressions_type;

      struct expression_type
      {
        ID_SIGNATURES(expression);
        PARENT_SIGNATURES(function);

      private:
        expressions_type _expressions;

      public:
        expression_type ( ID_CONS_PARAM(expression)
                        , PARENT_CONS_PARAM(function)
                        );
        expression_type ( ID_CONS_PARAM(expression)
                        , PARENT_CONS_PARAM(function)
                        , const expressions_type & exps
                        );

        void set (const std::string& exps);

        std::string expression (const std::string& sep = " ") const;

        const expressions_type& expressions (void) const;
        expressions_type& expressions (void);

        void append (const expressions_type& other);
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const expression_type & e
                  );
      }
    }
  }
}

#endif
