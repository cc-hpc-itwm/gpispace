// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_widget.hpp>

#include <QHBoxLayout>

#include <pnete/ui/graph_view.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      net_widget::net_widget ( graph::scene_type* scene
                             , QWidget* parent
                             )
        : QWidget (parent)
      {
        QHBoxLayout* layout (new QHBoxLayout());
        layout->addWidget (new graph_view (scene));
        layout->setContentsMargins (0, 0, 0, 0);
        setLayout (layout);
      }
    }
  }
}
