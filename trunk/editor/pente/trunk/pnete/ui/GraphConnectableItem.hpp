#ifndef GRAPHCONNECTABLEITEM_HPP
#define GRAPHCONNECTABLEITEM_HPP 1

#include <QGraphicsItem>
#include <QObject>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Connection;

      class ConnectableItem : public QObject, public QGraphicsItem
      {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

        public:
          enum eOrientation
          {
            NORTH = 0,
            EAST = 1,
            SOUTH = 2,
            WEST = 3,
            ANYORIENTATION = 4,
          };
          enum eDirection
          {
            IN = 0,
            OUT = 1,
            ANYDIRECTION = 2,
          };

          ConnectableItem(eOrientation orientation, eDirection direction, QGraphicsItem* parent = NULL);

          void connectMe(Connection* connection);
          void disconnectMe();

          const eOrientation& orientation() const;
          const eDirection& direction() const;

          void setOrientation(const eOrientation& orientation);

          virtual bool canConnectTo(ConnectableItem* other) const;
          virtual bool canConnectIn(eDirection thatDirection) const;

          bool createPendingConnectionIfPossible();

          const Connection* connection() const;

        protected:
          Connection* _connection;

          eDirection _direction;
          eOrientation _orientation;
      };
    }
  }
}

#endif
