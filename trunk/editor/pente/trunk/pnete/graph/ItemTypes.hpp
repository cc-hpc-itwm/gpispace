#ifndef GRAPHDEFINITIONS_HPP
#define GRAPHDEFINITIONS_HPP 1

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
        /* ... */
      };
    }
  }
}

#endif
