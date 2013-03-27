// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/expression.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/split.hpp> // fhg::util::lines
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
                                       , const util::position_type& pod
                                       , const expressions_type & exps
                                       )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
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

      void expression_type::append (const expressions_type& other)
      {
        expressions().insert
          (expressions().end(), other.begin(), other.end());
      }

      id::ref::expression expression_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return expression_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _expressions
          ).make_reference_id();
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
