#include <xml/parse/type/expression.hpp>

#include <util-generic/join.hpp>
#include <fhg/util/xml.hpp>

#include <stdexcept>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      namespace
      {
        void lines (const std::string& s, expressions_type& v)
        {
          char const sep (';');

          std::string::const_iterator pos (s.begin());
          std::string::const_iterator item_begin (s.begin());
          std::string::const_iterator item_end (s.begin());
          const std::string::const_iterator& end (s.end());

          while (pos != end)
          {
            if (*pos == sep)
            {
              v.push_back (std::string (item_begin, item_end));
              ++pos;
              while (pos != end && (*pos == sep || isspace (*pos)))
              {
                ++pos;
              }
              item_begin = item_end = pos;
            }
            else
            {
              ++item_end;
              ++pos;
            }
          }

          if (item_begin != item_end)
          {
            v.push_back (std::string (item_begin, item_end));
          }
        }

        expressions_type split (const expressions_type& exps)
        {
          expressions_type list;

          for ( expressions_type::const_iterator exp (exps.begin())
              ; exp != exps.end()
              ; ++exp
              )
          {
            lines (*exp, list);
          }

          return list;
        }
      }

      expression_type::expression_type ( const util::position_type& pod
                                       , const expressions_type & exps
                                       )
        : with_position_of_definition (pod)
        , _expressions (split (exps))
      {}

      void expression_type::set (const std::string& exps)
      {
        _expressions.clear();
        lines (exps, _expressions);
      }

      std::string expression_type::expression (const std::string& sep) const
      {
        return fhg::util::join (expressions(), ";" + sep).string();
      }

      const expressions_type& expression_type::expressions (void) const
      {
        return _expressions;
      }
      expressions_type& expression_type::expressions (void)
      {
        return _expressions;
      }

      void expression_type::append (const expressions_type& other)
      {
        expressions().insert
          (expressions().end(), other.begin(), other.end());
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
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
    }
  }
}
