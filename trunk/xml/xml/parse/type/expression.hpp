// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <string>
#include <iostream>
#include <list>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp>

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

      inline expressions_type split (const expressions_type& exps)
      {
        expressions_type list;

        for ( expressions_type::const_iterator exp (exps.begin())
            ; exp != exps.end()
            ; ++exp
            )
          {
            fhg::util::lines (*exp, ';', list);
          }

        return list;
      }

      struct expression_type
      {
      private:
        expressions_type _expressions;

      public:
        expression_type () : _expressions () {}

        expression_type (const expressions_type & exps)
          : _expressions (split (exps))
        {}

        void set (const std::string& exps)
        {
          _expressions.clear();
          fhg::util::lines (exps, ';', _expressions);
        }

        std::string expression (const std::string& sep = " ") const
        {
          return fhg::util::join ( expressions().begin()
                                 , expressions().end()
                                 , ";" + sep
                                 );
        }

        const expressions_type& expressions (void) const
        {
          return _expressions;
        }
        expressions_type& expressions (void)
        {
          return _expressions;
        }
      };

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const expression_type & e
                         )
        {
          s.open ("expression");

          const std::string exp (e.expression ());

          if (exp.size() > 0)
            {
              s.content (exp);
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
            x.expressions().insert ( x.expressions().end()
                                   , e.expressions().begin()
                                   , e.expressions().end()
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
