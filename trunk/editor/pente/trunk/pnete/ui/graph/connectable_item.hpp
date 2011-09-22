#ifndef _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP
#define _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP 1

#include <QObject>

#include <pnete/ui/graph/item.hpp>

class QGraphicsItem;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connection;

        class connectable_item : public item
        {
          Q_OBJECT;

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

          connectable_item ( eOrientation orientation
                           , eDirection direction
                           , QGraphicsItem* parent = NULL
                           );

          void connectMe (connection* connection);
          void disconnectMe();

          const eOrientation& orientation() const;
          const eDirection& direction() const;

          void setOrientation (const eOrientation& orientation);

          bool canConnectTo (connectable_item* other) const;
          bool canConnectIn (eDirection thatDirection) const;

          bool createPendingConnectionIfPossible();

          const connection* get_connection() const;

          const QString& we_type() const;
          const QString& we_type (const QString&);

        signals:
          void we_type_changed();

        protected:
          connection* _connection;

          eDirection _direction;
          eOrientation _orientation;

          QString _we_type;
        };
      }
    }
  }
}

#endif
