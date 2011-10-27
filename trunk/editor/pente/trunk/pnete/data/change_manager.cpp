// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

#include <we/expr/parse/parser.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      change_manager_t::change_manager_t (internal_type & i)
        : _internal (i)
      {}

      void change_manager_t::set_function_name
      ( const QObject* origin
      , ::xml::parse::type::function_type& fun
      , const QString& name
      )
      {
        if (!name.isEmpty())
        {
          fun.name = name.toStdString();
        }
        else
        {
          fun.name.clear();
        }

        emit signal_set_function_name (origin, fun, name);
      }

      void change_manager_t::set_expression
      ( const QObject* origin
      , ::xml::parse::type::expression_type& expression
      , const QString& text
      )
      {
        expression.set (text.toStdString());

        emit signal_set_expression (origin, expression, text);

        emit signal_set_expression_parse_result
          ( origin
          , expression
          , QString::fromStdString
            (expr::parse::parse_result (text.toStdString()))
          );
      }

      void change_manager_t::delete_transition
      ( const ::xml::parse::type::transition_type& trans
      , ::xml::parse::type::net_type& net
      )
      {
        //! \todo inform the world *before* deleting the transition
        net.erase_transition (trans);
      }
    }
  }
}
