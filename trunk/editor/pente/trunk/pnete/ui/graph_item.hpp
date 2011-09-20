// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_GRAPH_GRAPH_ITEM_HPP
#define UI_GRAPH_GRAPH_ITEM_HPP 1

#include <QObject>
#include <QGraphicsItem>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/util.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      namespace GraphViz
      {
        class Graph;
      }
      class graph_item : public QObject, public QGraphicsItem
      {
        Q_OBJECT;
        Q_INTERFACES(QGraphicsItem);

      public:
        enum ItemTypes
          {
            ConnectionType    = QGraphicsItem::UserType + 1,
            PortType          = QGraphicsItem::UserType + 2,
            TransitionType    = QGraphicsItem::UserType + 3,
            ParameterPortType = QGraphicsItem::UserType + 4,
            place_type        = QGraphicsItem::UserType + 5,
            /* ... */
          };

        graph::Scene* scene() const
        {
          return
            util::throwing_qobject_cast<graph::Scene*> (QGraphicsItem::scene());
        }

        graph_item (QGraphicsItem* parent = NULL) : QGraphicsItem (parent) { }

        virtual void add_to_graphviz_graph (GraphViz::Graph & graph) const { }
      };
    }
  }
}

#endif
