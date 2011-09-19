// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_widget.hpp>

#include <QWidget>

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
          , _view (new ui::GraphView (scene, this))
      {
        //! \todo Size policy or whatever to have graph view filling the widget.
        _view->setScene (scene);
      }
    }
  }
}
