// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_NET_WIDGET_HPP
#define _PNETE_UI_NET_WIDGET_HPP 1

#include <pnete/ui/graph/scene.fwd.hpp>

#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class net_widget : public QWidget
      {
        Q_OBJECT;

      public:
        net_widget (graph::scene_type* scene, QWidget* parent = NULL);
      };
    }
  }
}

#endif
