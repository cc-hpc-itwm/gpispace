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

      expression_type::expression_type ( const id::expression& id
                                       , const id::function& parent
                                       , id::mapper* id_mapper
                                       )
        : _expressions()
        , _id (id)
        , _parent (parent)
        , _id_mapper (id_mapper)
      {
        _id_mapper->put (_id, *this);
      }

      expression_type::expression_type ( const expressions_type & exps
                                       , const id::expression& id
                                       , const id::function& parent
                                       , id::mapper* id_mapper
                                       )
        : _expressions (split (exps))
        , _id (id)
        , _parent (parent)
        , _id_mapper (id_mapper)
      {
        _id_mapper->put (_id, *this);
      }


      const id::expression& expression_type::id() const
      {
        return _id;
      }
      const id::function& expression_type::parent() const
      {
        return _parent;
      }

      bool expression_type::is_same (const expression_type& other) const
      {
        return id() == other.id() && parent() == other.parent();
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
          const expression_type & e;

        public:
          join_visitor (const expression_type & _e) : e(_e) {}

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

      void join (const expression_type& e, function_type& fun)
      {
        boost::apply_visitor (join_visitor (e), fun.f);
      }
    }
  }
}
