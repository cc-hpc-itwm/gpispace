#ifndef GRAPHSCENE_HPP
#define GRAPHSCENE_HPP 1

#include <pnete/data/internal.hpp>

#include <xml/parse/types.hpp>

#include <QGraphicsScene>
#include <QPointF>
#include <QObject>
#include <QMenu>

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QRectF;
class QMenu;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connection;
        class connectable_item;

        class scene : public QGraphicsScene
        {
          Q_OBJECT;

        public:
          typedef ::xml::parse::type::net_type net_type;

          explicit scene (net_type & net,  QObject* parent = NULL);

          const QPointF& mousePosition() const;

          void setPendingConnection (connection* connection);
          void removePendingConnection();

          const connection* pendingConnection() const;
          bool pendingConnectionCanConnectTo (connectable_item* item) const;
          void pendingConnectionConnectTo (connectable_item* item);
          bool createPendingConnectionWith (connectable_item* item);

          QString name() const;

          void create_connection ( connectable_item* from
                                 , connectable_item* to
                                 , bool only_reading
                                 );

        public slots:
          void slot_add_transition ();
          void slot_add_place ();
          void slot_add_struct ();

          void auto_layout();

        protected:
          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent(QKeyEvent* event);

        private:
          connection* _pendingConnection;
          QPointF _mousePosition;

          QMenu _menu_new;
          QMenu _menu_context;

          net_type & _net;

          void init_menu_context();
        };
      }
    }
  }
}

#endif
