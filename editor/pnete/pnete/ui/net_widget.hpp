// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_NET_WIDGET_HPP
#define _PNETE_UI_NET_WIDGET_HPP 1

#include <pnete/data/proxy.hpp>
#include <pnete/ui/base_editor_widget.hpp>

#include <QObject>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class scene_type;
      }
    }

    namespace ui
    {
      class net_widget : public base_editor_widget
      {
        Q_OBJECT;

      public:
        net_widget ( data::proxy::type& proxy
                   , graph::scene_type* scene
                   , QWidget* parent = NULL
                   );
      };
    }
  }
}

#endif
