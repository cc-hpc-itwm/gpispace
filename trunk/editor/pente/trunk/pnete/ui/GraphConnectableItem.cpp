#include <pnete/ui/GraphConnectableItem.hpp>
#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphConnection.hpp>

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
      , _we_type (tr ("<<we_type>>"))
      {}

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
        return we_type() == other->we_type()
            && canConnectIn (other->direction());
      }
      bool ConnectableItem::canConnectIn (eDirection thatDirection) const
      {
        return thatDirection != direction();
      }

      bool ConnectableItem::createPendingConnectionIfPossible()
      {
        Scene* sceneObject (scene());

        if (sceneObject->pendingConnection())
        {
          return false;
        }

//         if (_connection)
//         {
//           sceneObject->setPendingConnection (_connection);
//           _connection->removeMe (this);
//           return true;
//         }
//        else
        {
          return sceneObject->createPendingConnectionWith (this);
        }
      }

      //! \ŧodo mousehandler

      void ConnectableItem::setOrientation (const eOrientation& orientation)
      {
        _orientation = orientation;
      }

      const Connection* ConnectableItem::connection() const
      {
        return _connection;
      }

      const QString& ConnectableItem::we_type() const
      {
        return _we_type;
      }
      const QString& ConnectableItem::we_type(const QString& we_type_)
      {
        _we_type = we_type_;
        emit we_type_changed();
        return _we_type;
      }
    }
  }
}
