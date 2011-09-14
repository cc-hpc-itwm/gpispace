#include "GraphConnectableItem.hpp"
#include "GraphScene.hpp"
#include "GraphConnection.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      ConnectableItem::ConnectableItem ( eOrientation orientation
                                       , eDirection direction
                                       , QGraphicsItem* parent
                                       )
      : graph_item (parent)
      , _connection (NULL)
      , _direction (direction)
      , _orientation (orientation)
      {
      }

      void ConnectableItem::connectMe (Connection* connection)
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

      bool ConnectableItem::canConnectTo (ConnectableItem* other) const
      {
        return true;
      }
      bool ConnectableItem::canConnectIn (eDirection thatDirection) const
      {
        return true;
      }

      bool ConnectableItem::createPendingConnectionIfPossible()
      {
        Scene* sceneObject = qobject_cast<Scene*> (scene());

        if (sceneObject->pendingConnection())
        {
          return false;
        }

        if (_connection)
        {
          sceneObject->setPendingConnection (_connection);
          _connection->removeMe (this);
          return true;
        }
        else
        {
          return sceneObject->createPendingConnectionWith (this);
        }
      }

      void ConnectableItem::setOrientation (const eOrientation& orientation)
      {
        _orientation = orientation;
      }

      const Connection* ConnectableItem::connection() const
      {
        return _connection;
      }
    }
  }
}
