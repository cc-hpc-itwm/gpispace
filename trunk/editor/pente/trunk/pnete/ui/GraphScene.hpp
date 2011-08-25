#ifndef GRAPHSCENE_HPP
#define GRAPHSCENE_HPP 1

#include <QGraphicsScene>
#include <QPointF>
#include <QObject>

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QRectF;

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
          Scene(QObject* parent = NULL);
          Scene(const QRectF& sceneRect, QObject* parent = NULL);

          const QPointF& mousePosition() const;

          void setPendingConnection(Connection* connection);
          void removePendingConnection();

          const Connection* pendingConnection() const;
          bool pendingConnectionCanConnectTo(ConnectableItem* item) const;
          void pendingConnectionConnectTo(ConnectableItem* item);
          bool createPendingConnectionWith(ConnectableItem* item);

        protected:
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent(QKeyEvent* event);

        private:
          Connection* _pendingConnection;
          QPointF _mousePosition;
      };
    }
  }
}

#endif
