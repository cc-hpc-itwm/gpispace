// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_NET_WIDGET_HPP
#define _PNETE_UI_NET_WIDGET_HPP 1

#include <QObject>
#include <QGraphicsView>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/base_editor_widget.hpp>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class GraphView;

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
                   , data::proxy::net_proxy::data_type& net
                   , graph::scene_type* scene
                   , const QStringList& types
                   , QWidget* parent = NULL
                   );

      private:
        data::proxy::net_proxy::data_type& _net;
        ui::GraphView* _view;
      };
    }
  }
}

#endif
