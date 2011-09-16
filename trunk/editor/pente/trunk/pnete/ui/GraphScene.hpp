#ifndef GRAPHSCENE_HPP
#define GRAPHSCENE_HPP 1

#include <pnete/data/internal.hpp>

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
    namespace graph
    {
      class Connection;
      class ConnectableItem;

      class Scene : public QGraphicsScene
      {
        Q_OBJECT

        public:
          explicit Scene (data::internal::ptr data, QObject* parent = NULL);

          const QPointF& mousePosition() const;

          void setPendingConnection(Connection* connection);
          void removePendingConnection();

          const Connection* pendingConnection() const;
          bool pendingConnectionCanConnectTo(ConnectableItem* item) const;
          void pendingConnectionConnectTo(ConnectableItem* item);
          bool createPendingConnectionWith(ConnectableItem* item);

          void save (const QString& filename) const;

          QString name() const;

        public slots:
          void slot_add_transition ();
          void slot_add_place ();
          void slot_add_struct ();

          void auto_layout();

        protected:
          virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent(QKeyEvent* event);

        private:
          Connection* _pendingConnection;
          QPointF _mousePosition;

          QMenu _menu_new;
          QMenu _menu_context;

          data::internal::ptr _data;

          void init_menu_context();
      };
    }
  }
}

#endif
