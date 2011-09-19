// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/base_editor_widget.hpp>

#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      base_editor_widget::base_editor_widget
        (data::proxy::type& proxy, QWidget* parent)
          : QWidget (parent)
          , _proxy (proxy)
      {}

      data::proxy::type& base_editor_widget::proxy () const { return _proxy; }
    }
  }
}
