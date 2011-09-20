#ifndef GRAPHCONNECTABLEITEM_HPP
#define GRAPHCONNECTABLEITEM_HPP 1

#include <QObject>

#include <pnete/ui/graph_item.hpp>

class QGraphicsItem;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Connection;

      class ConnectableItem : public graph_item
      {
        Q_OBJECT

        public:
          enum eOrientation
          {
            NORTH = 1 << 0,
            EAST = 1 << 1,
            SOUTH = 1 << 2,
            WEST = 1 << 3,
            ANYORIENTATION = NORTH | EAST | SOUTH | WEST,
          };
          enum eDirection
          {
            IN = 1 << 0,
            OUT = 1 << 1,
            ANYDIRECTION = IN | OUT,
          };

          ConnectableItem(eOrientation orientation, eDirection direction, QGraphicsItem* parent = NULL);

          void connectMe(Connection* connection);
          void disconnectMe();

          const eOrientation& orientation() const;
          const eDirection& direction() const;

          void setOrientation(const eOrientation& orientation);

          bool canConnectTo(ConnectableItem* other) const;
          bool canConnectIn(eDirection thatDirection) const;

          bool createPendingConnectionIfPossible();

          const Connection* connection() const;

          const QString& we_type() const;
          const QString& we_type(const QString&);

        signals:
          void we_type_changed();

        protected:
          Connection* _connection;

          eDirection _direction;
          eOrientation _orientation;

          QString _we_type;
      };
    }
  }
}

#endif
