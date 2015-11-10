#pragma once

#include <xml/parse/type/expression.fwd.hpp>

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
      public:
        expression_type ( const util::position_type&
                        , const expressions_type& exps
                        = expressions_type()
                        );

        void set (const std::string& exps);

        std::string expression (const std::string& sep = " ") const;

        const expressions_type& expressions (void) const;
        expressions_type& expressions (void);

        void append (const expressions_type& other);

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
