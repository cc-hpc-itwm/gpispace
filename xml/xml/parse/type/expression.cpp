// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/expression.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/function.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp> // fhg::util::lines
#include <fhg/util/xml.hpp>

#include <stdexcept>

#include <boost/variant.hpp>

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

      expression_type::expression_type ( ID_CONS_PARAM(expression)
                                       , PARENT_CONS_PARAM(function)
                                       )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _expressions()
      {
        _id_mapper->put (_id, *this);
      }

      expression_type::expression_type ( ID_CONS_PARAM(expression)
                                       , PARENT_CONS_PARAM(function)
                                       , const expressions_type & exps
                                       )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _expressions (split (exps))
      {
        _id_mapper->put (_id, *this);
      }

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

      namespace
      {
        class join_visitor : public boost::static_visitor<void>
        {
        private:
          const expressions_type& _es;
          id::mapper* _id_mapper;

        public:
          join_visitor ( const expressions_type & es
                       , id::mapper* id_mapper
                       )
            : _es (es)
            , _id_mapper (id_mapper)
          {}

          void operator () (id::ref::expression & id_expression) const
          {
            boost::optional<expression_type&>
              expression (_id_mapper->get_ref (id_expression));

            expression->expressions().insert
              (expression->expressions().end(), _es.begin(), _es.end());
          }

          template<typename T>
          void operator () (T &) const
          {
            throw std::runtime_error ("BUMMER: join for non expression!");
          }
        };
      }

      void join (const expressions_type& es, function_type& fun)
      {
        boost::apply_visitor (join_visitor (es, fun.id_mapper()), fun.f);
      }
    }
  }
}
