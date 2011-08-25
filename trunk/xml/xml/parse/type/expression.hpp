// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <string>
#include <iostream>
#include <list>

#include <fhg/util/join.hpp>

#include <boost/variant.hpp>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::list<std::string> expressions_type;

      struct expression_type
      {
        expressions_type expressions;

        expression_type () : expressions () {}

        expression_type (const expressions_type & _expressions)
          : expressions (_expressions)
        {}

        std::string expression (void) const
        {
          return fhg::util::join (expressions.begin(), expressions.end(), "; ");
        }
      };

      namespace dump
      {
        inline void dump (xml_util::xmlstream & s, const expression_type & e)
        {
          s.open ("expression");

          if (e.expression().size() > 0)
            {
              s.content (e.expression());
            }

          s.close ();
        }
      }

      namespace visitor
      {
        class join : public boost::static_visitor<void>
        {
        private:
          const expression_type & e;

        public:
          join (const expression_type & _e) : e(_e) {}

          void operator () (expression_type & x) const
          {
            x.expressions.insert ( x.expressions.end()
                                 , e.expressions.begin()
                                 , e.expressions.end()
                                 );
          }

          template<typename T>
          void operator () (T &) const
          {
            throw std::runtime_error ("BUMMER: join for non expression!");
          }
        };
      }
    }
  }
}

#endif
