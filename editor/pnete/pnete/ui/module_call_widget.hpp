// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_MODULE_CALL_WIDGET_HPP
#define _PNETE_UI_MODULE_CALL_WIDGET_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/base_editor_widget.hpp>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class port_lists_widget;

      class module_call_widget : public base_editor_widget
      {
        Q_OBJECT;

      public:
        module_call_widget ( data::proxy::type& proxy
                           , const ::xml::parse::id::ref::module& mod
                           , const QStringList& types
                           , QWidget* parent = NULL
                           );
      };
    }
  }
}

#endif
