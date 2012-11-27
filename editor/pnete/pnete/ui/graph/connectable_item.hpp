// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP
#define PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP

#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connection.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>

#include <QObject>
#include <QSet>
#include <QRectF>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
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
            , base_item* parent = NULL
            );

          void add_connection (connection_item* c);
          void remove_connection (connection_item * c);

          virtual bool is_connectable_with (const connectable_item*) const;

          const QSet<connection_item*>& connections() const;
          const connectable::direction::type& direction() const;
          const connectable::direction::type& direction (const connectable::direction::type&);

          virtual const std::string& we_type() const = 0;

          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

          virtual QLinkedList<base_item*> childs() const;

        protected:
          QSet<connection_item*> _connections;
          connectable::direction::type _direction;

          const std::string& we_type (const std::string&) const;
        };
      }
    }
  }
}

#endif
