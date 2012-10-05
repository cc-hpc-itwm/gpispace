#include <pnete/ui/graph/connectable_item.hpp>

#include <QDebug>

#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/scene.hpp>

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
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , graph::item* parent
          , ::we::type::property::type* property
          )
          : graph::item (parent, property)
          , _connections ()
          , _direction (direction)
          , _type_map (type_map)
        {}

        void connectable_item::add_connection (connection::item* c)
        {
          if (_connections.contains (c))
          {
            throw std::runtime_error ("tried adding the same connection twice.");
          }
          _connections.insert (c);
        }
        void connectable_item::remove_connection (connection::item* c)
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

        const QSet<connection::item*>& connectable_item::connections() const
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
          if (_type_map)
          {
            ::xml::parse::type::type_map_type::const_iterator type_mapping
              (_type_map->find (unmapped));

            if (type_mapping != _type_map->end())
            {
              return type_mapping->second;
            }
          }

          return unmapped;
        }

        QLinkedList<graph::item*> connectable_item::childs() const
        {
          QLinkedList<graph::item*> childs;

          foreach (connection::item* connection, connections())
          {
            childs << connection;
          }

          return childs;
        }

        void connectable_item::erase_connections (scene::type* scene)
        {
          foreach (connection::item* connection, connections())
          {
            scene->removeItem (connection);

            if (connection)
            {
              delete connection;
            }
          }
        }

//      void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
//      {
//        scene()->create_connection (this);
//        //! \todo Start a new connection!
//      }
      }
    }
  }
}
