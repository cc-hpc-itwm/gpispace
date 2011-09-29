#include <pnete/ui/graph/connectable_item.hpp>

#include <QDebug>

#include <pnete/ui/graph/connection.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connectable_item::connectable_item
          ( DIRECTION direction
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , item* parent
          )
            : item (parent)
            , _connections ()
            , _direction (direction)
            , _we_type (tr ("<<we_type>>"))
            , _type_map (type_map)
        {}

        void connectable_item::add_connection (connection* c)
        {
          if (_connections.contains (c))
          {
            throw std::runtime_error ("tried adding the same connection twice.");
          }
          _connections.insert (c);
        }
        void connectable_item::remove_connection (connection* c)
        {
          if (!_connections.contains (c))
          {
            throw std::runtime_error ("item did not have that connection.");
          }
          _connections.remove (c);
        }

        bool
        connectable_item::is_connectable_with (const connectable_item* i) const
        {
          return i->we_type() == we_type() && i->direction() != direction();
        }

        const QSet<connection*>& connectable_item::connections() const
        {
          return _connections;
        }

        const connectable_item::DIRECTION&
        connectable_item::direction() const
        {
          return _direction;
        }
        const connectable_item::DIRECTION&
        connectable_item::direction (const DIRECTION& direction_)
        {
          return _direction = direction_;
        }

        QString connectable_item::we_type() const
        {
          if (_type_map)
            {
              ::xml::parse::type::type_map_type::const_iterator type_mapping
                (_type_map->find (_we_type.toStdString()));

              if (type_mapping != _type_map->end())
                {
                  return QString::fromStdString (type_mapping->second);
                }
            }

          return _we_type;
        }
        const QString& connectable_item::we_type (const QString& we_type_)
        {
          _we_type = we_type_;
          emit we_type_changed();
          //! \todo check, if connections to this item are still valid!
          return _we_type;
        }

        void connectable_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          scene()->create_connection (this);
          //! \todo Start a new connection!
        }
      }
    }
  }
}
