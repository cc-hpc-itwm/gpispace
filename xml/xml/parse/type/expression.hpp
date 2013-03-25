// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <xml/parse/type/expression.fwd.hpp>

#include <xml/parse/id/generic.hpp>
#include <xml/parse/id/types.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      //! \todo Move this into class scope.
      typedef std::list<std::string> expressions_type;

      struct expression_type : with_position_of_definition
      {
        ID_SIGNATURES(expression);
        PARENT_SIGNATURES(function);

      public:
        expression_type ( ID_CONS_PARAM(expression)
                        , PARENT_CONS_PARAM(function)
                        , const util::position_type&
                        , const expressions_type& exps
                        = expressions_type()
                        );

        void set (const std::string& exps);

        std::string expression (const std::string& sep = " ") const;

        const expressions_type& expressions (void) const;
        expressions_type& expressions (void);

        void append (const expressions_type& other);

        id::ref::expression clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        expressions_type _expressions;
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
