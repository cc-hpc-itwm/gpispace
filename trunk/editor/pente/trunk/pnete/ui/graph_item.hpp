// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_GRAPH_GRAPH_ITEM_HPP
#define UI_GRAPH_GRAPH_ITEM_HPP 1

#include <QObject>
#include <QGraphicsItem>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      enum ItemTypes
      {
        ConnectionType    = QGraphicsItem::UserType + 1,
        PortType          = QGraphicsItem::UserType + 2,
        TransitionType    = QGraphicsItem::UserType + 3,
        ParameterPortType = QGraphicsItem::UserType + 4,
        /* ... */
      };
      namespace GraphViz
      {
        class Graph;
      }
      class graph_item : public QObject, public QGraphicsItem
      {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

        public:
          graph_item (QGraphicsItem* parent = NULL) : QGraphicsItem (parent) { }

          virtual void add_to_graphviz_graph (GraphViz::Graph & graph) const { }
      };
    }
  }
}

#endif
