// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP
#define _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP 1

#include <QObject>
#include <QSet>
#include <QRectF>

#include <pnete/ui/graph/item.hpp>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace connection { class item; }
        namespace scene { class type; }

        namespace connectable
        {
          namespace direction
          {
            enum type
              { IN   = 1 << 0
              , OUT  = 1 << 1
              , BOTH = IN | OUT
              };
          }

          class item : public graph::item
          {
            Q_OBJECT;

          public:
            item
            ( direction::type direction
            , boost::optional< ::xml::parse::type::type_map_type&>
            = boost::none
            , graph::item* parent = NULL
            , ::we::type::property::type* property = NULL
            );

            void add_connection (connection::item* c);
            void remove_connection (connection::item * c);

            virtual bool is_connectable_with (const item*) const;

            void erase_connections (scene::type*);
            const QSet<connection::item*>& connections() const;
            const direction::type& direction() const;
            const direction::type& direction (const direction::type&);

            virtual const std::string& we_type() const = 0;

//             virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

            virtual QLinkedList<graph::item*> childs() const;

          protected:
            QSet<connection::item*> _connections;
            direction::type _direction;
            boost::optional< ::xml::parse::type::type_map_type&> _type_map;

            const std::string& we_type (const std::string&) const;
          };
        }
      }
    }
  }
}

#endif
