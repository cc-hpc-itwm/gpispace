// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_widget.hpp>

#include <QWidget>
#include <QHBoxLayout>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphView.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      net_widget::net_widget
        ( data::proxy::type& proxy
        , data::proxy::net_proxy::data_type& net
        , graph::Scene* scene
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _net (net)
          , _view (new ui::GraphView (scene))
      {
        QHBoxLayout* layout (new QHBoxLayout());
        layout->addWidget (_view);
        layout->setContentsMargins (0, 0, 0, 0);
        setLayout (layout);
        _view->setScene (scene);
      }
    }
  }
}
