#include "ConnectableItem.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      ConnectableItem::ConnectableItem(eOrientation orientation, eDirection direction, QGraphicsItem* parent)
      : QGraphicsItem(parent),
      _connection(NULL),
      _direction(direction),
      _orientation(orientation)
      {
      }
      
      const QGraphicsItem* ConnectableItem::toQGraphicsItem() const
      {
        return static_cast<const QGraphicsItem*>(this);
      }
      
      void ConnectableItem::connectMe(Connection* connection)
      {
        _connection = connection;
      }
      void ConnectableItem::disconnectMe()
      {
        _connection = NULL;
      }
      
      const ConnectableItem::eOrientation& ConnectableItem::orientation() const
      {
        return _orientation;
      }
      const ConnectableItem::eDirection& ConnectableItem::direction() const
      {
        return _direction;
      }
    }
  }
}
