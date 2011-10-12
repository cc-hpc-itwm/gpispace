// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP
#define _PNETE_UI_GRAPH_CONNECTABLE_ITEM_HPP 1

#include <QObject>
#include <QSet>

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

        namespace connectable
        {
          class item : public graph::item
          {
            Q_OBJECT;

          public:
            enum DIRECTION
              {
                IN = 1 << 0,
                  OUT = 1 << 1,
                  BOTH = IN | OUT,
              };

            item
            ( DIRECTION direction
            , boost::optional< ::xml::parse::type::type_map_type&>
            = boost::none
            , graph::item* parent = NULL
            );

            void add_connection (connection::item* c);
            void remove_connection (connection::item * c);

            virtual bool is_connectable_with (const item*) const;

            const QSet<connection::item*>& connections() const;
            const DIRECTION& direction() const;
            const DIRECTION& direction (const DIRECTION&);
            QString we_type() const;
            const QString& we_type (const QString&);

            virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

          signals:
            void we_type_changed();

          protected:
            QSet<connection::item*> _connections;
            DIRECTION _direction;
            QString _we_type;
            boost::optional< ::xml::parse::type::type_map_type&> _type_map;
          };
        }
      }
    }
  }
}

#endif
