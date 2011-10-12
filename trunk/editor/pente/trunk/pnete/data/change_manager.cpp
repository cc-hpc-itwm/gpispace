// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

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
        expression.expressions.clear();
        expression.expressions.push_back (text.toStdString());

        emit signal_set_expression (origin, expression, text);
      }
    }
  }
}
