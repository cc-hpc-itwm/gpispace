// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <xml/parse/type/function.fwd.hpp>

#include <xml/parse/type/id.hpp>

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

      private:
        expressions_type _expressions;

        id::function _parent;

      public:
        expression_type ( ID_CONS_PARAM(expression)
                        , const id::function& parent
                        );
        expression_type ( ID_CONS_PARAM(expression)
                        , const expressions_type & exps
                        , const id::function& parent
                        );

        const id::function& parent() const;

        void set (const std::string& exps);

        std::string expression (const std::string& sep = " ") const;

        const expressions_type& expressions (void) const;
        expressions_type& expressions (void);
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const expression_type & e
                  );
      }

      void join (const expression_type& e, function_type& fun);
    }
  }
}

#endif
