// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connectable_item.hpp>

#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/scene.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connectable_item::connectable_item
          ( connectable::direction::type direction
          , base_item* parent
          )
          : base_item (parent)
          , _connections ()
          , _direction (direction)
        {}

        void connectable_item::add_connection (connection_item* c)
        {
          if (_connections.contains (c))
          {
            throw std::runtime_error ("tried adding the same connection twice.");
          }
          _connections.insert (c);
        }
        void connectable_item::remove_connection (connection_item* c)
        {
          if (!_connections.contains (c))
          {
            throw std::runtime_error ("item did not have that connection.");
          }
          _connections.remove (c);
        }

        bool connectable_item::is_connectable_with (const connectable_item* i) const
        {
          return i->we_type() == we_type() && i->direction() != direction();
        }

        const QSet<connection_item*>& connectable_item::connections() const
        {
          return _connections;
        }

        const connectable::direction::type& connectable_item::direction() const
        {
          return _direction;
        }
        const connectable::direction::type& connectable_item::direction (const connectable::direction::type& direction_)
        {
          return _direction = direction_;
        }

        const std::string& connectable_item::we_type (const std::string& unmapped) const
        {
          //! \todo Reimplement with getting type_map at call-time from handle.
          // - get type_map
          // - it = type_map.find (unmapped)
          // - it ? return it->mapped_type : unmapped
          return unmapped;
        }

        QLinkedList<base_item*> connectable_item::childs() const
        {
          QLinkedList<base_item*> childs;

          foreach (connection_item* connection, connections())
          {
            childs << connection;
          }

          return childs;
        }

        //! \todo This should be in the different connectable items instead.
        void connectable_item::mousePressEvent
          (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ShiftModifier)
          {
            //! \todo Ports can start a connection when they are
            //! already connected to something!
            scene()->create_pending_connection (this);
          }
          else
          {
            base_item::mousePressEvent (event);
          }
        }
      }
    }
  }
}
