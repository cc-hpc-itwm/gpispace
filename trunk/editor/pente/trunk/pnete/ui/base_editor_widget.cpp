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
        ( data::proxy::type& proxy
        , const QString& fallback_title
        , QWidget* parent
        )
          : QWidget (parent)
          , _proxy (proxy)
          , _fallback_title (fallback_title)
      {}

      data::proxy::type& base_editor_widget::proxy () const { return _proxy; }

      QString base_editor_widget::name () const
      {
        return data::proxy::name (proxy(), _fallback_title);
      }
    }
  }
}
