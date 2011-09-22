#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/connection.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connectable_item::connectable_item ( eOrientation orientation
                                           , eDirection direction
                                           , QGraphicsItem* parent
                                           )
          : item (parent)
          , _connection (NULL)
          , _direction (direction)
          , _orientation (orientation)
          , _we_type (tr ("<<we_type>>"))
        {}

        void connectable_item::connectMe (connection* c)
        {
          _connection = c;
        }
        void connectable_item::disconnectMe()
        {
          _connection = NULL;
        }

        const connectable_item::eOrientation&
        connectable_item::orientation() const
        {
          return _orientation;
        }
        const connectable_item::eDirection& connectable_item::direction() const
        {
          return _direction;
        }

        bool connectable_item::canConnectTo (connectable_item* other) const
        {
          return we_type() == other->we_type()
            && canConnectIn (other->direction());
        }
        bool connectable_item::canConnectIn (eDirection thatDirection) const
        {
          return thatDirection != direction();
        }

        bool connectable_item::createPendingConnectionIfPossible()
        {
          class scene* sceneObject (scene());

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

        //! \todo mousehandler

        void connectable_item::setOrientation (const eOrientation& orientation)
        {
          _orientation = orientation;
        }

        const connection* connectable_item::get_connection() const
        {
          return _connection;
        }

        const QString& connectable_item::we_type() const
        {
          return _we_type;
        }
        const QString& connectable_item::we_type(const QString& we_type_)
        {
          _we_type = we_type_;
          emit we_type_changed();
          return _we_type;
        }
      }
    }
  }
}
