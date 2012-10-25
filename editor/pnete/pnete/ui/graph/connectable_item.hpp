// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP
#define _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP 1

#include <QObject>
#include <QSet>
#include <QRectF>

#include <pnete/ui/graph/base_item.hpp>

#include <boost/optional.hpp>

#include <xml/parse/type_map_type.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connection_item;
        class scene_type;

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
        }

        class connectable_item : public base_item
        {
          Q_OBJECT;

        public:
          connectable_item
            ( connectable::direction::type direction
            , boost::optional< ::xml::parse::type::type_map_type&>
            = boost::none
            , base_item* parent = NULL
            , ::we::type::property::type* property = NULL
            );

          void add_connection (connection_item* c);
          void remove_connection (connection_item * c);

          virtual bool is_connectable_with (const connectable_item*) const;

          void erase_connections (scene_type*);
          const QSet<connection_item*>& connections() const;
          const connectable::direction::type& direction() const;
          const connectable::direction::type& direction (const connectable::direction::type&);

          virtual const std::string& we_type() const = 0;

//        virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

          virtual QLinkedList<base_item*> childs() const;

        protected:
          QSet<connection_item*> _connections;
          connectable::direction::type _direction;
          boost::optional< ::xml::parse::type::type_map_type&> _type_map;

          const std::string& we_type (const std::string&) const;
        };
      }
    }
  }
}

#endif
