// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/expression.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp> // fhg::util::lines
#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      namespace
      {
        expressions_type split (const expressions_type& exps)
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
      }

      expression_type::expression_type () : _expressions () {}

      expression_type::expression_type (const expressions_type & exps)
        : _expressions (split (exps))
      {}

      void expression_type::set (const std::string& exps)
      {
        _expressions.clear();
        fhg::util::lines (exps, ';', _expressions);
      }

      std::string expression_type::expression (const std::string& sep) const
      {
        return fhg::util::join ( expressions().begin()
                               , expressions().end()
                               , ";" + sep
                               );
      }

      const expressions_type& expression_type::expressions (void) const
      {
        return _expressions;
      }
      expressions_type& expression_type::expressions (void)
      {
        return _expressions;
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

      // namespace
      // {
      //   class join_visitor : public boost::static_visitor<void>
      //   {
      //   private:
      //     const expression_type & e;

      //   public:
      //     join_visitor (const expression_type & _e) : e(_e) {}

      //     void operator () (expression_type & x) const
      //     {
      //       x.expressions().insert ( x.expressions().end()
      //                              , e.expressions().begin()
      //                              , e.expressions().end()
      //                              );
      //     }

      //     template<typename T>
      //     void operator () (T &) const
      //     {
      //       throw std::runtime_error ("BUMMER: join for non expression!");
      //     }
      //   };
      // }

      // void join (const expression_type& e, function_type::type& fun)
      // {
      //   boost::apply_visitor (join_visitor (e), f);
      // }
    }
  }
}
