// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <QWidget>

#include <pnete/ui/expression_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_widget::expression_widget
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type& expression
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _expression (expression)
      {}
    }
  }
}
