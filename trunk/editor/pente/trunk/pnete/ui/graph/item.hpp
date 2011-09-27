// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_ITEM_HPP
#define _PNETE_UI_GRAPH_ITEM_HPP 1

#include <QObject>
#include <QGraphicsItem>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/util.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class item : public QObject, public QGraphicsItem
        {
          Q_OBJECT;
          Q_INTERFACES(QGraphicsItem);

        public:
          enum ItemTypes
          {
            connection_graph_type       = QGraphicsItem::UserType + 1,
            port_graph_type             = QGraphicsItem::UserType + 2,
            transition_graph_type       = QGraphicsItem::UserType + 3,
            place_graph_type            = QGraphicsItem::UserType + 4,
            /* ... */
          };


          class scene* scene() const
          {
            return
              util::throwing_qobject_cast<class scene*>(QGraphicsItem::scene());
          }

          item (item* parent = NULL) : QGraphicsItem (parent) { }
        };
      }
    }
  }
}

#endif
