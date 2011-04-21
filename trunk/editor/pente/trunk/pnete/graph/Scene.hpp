#ifndef GRAPHSCENE_HPP
#define GRAPHSCENE_HPP 1

#include <QGraphicsScene>
#include <QPointF>
#include <QObject>

class QGraphicsSceneMouseEvent;
class QKeyEvent;

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
          
          const QPointF& mousePosition() const;
          
          //! \todo rename to pendingConnection.
          const Connection* newConnection() const;
          void setNewConnection(Connection* newConnection);
          void addConnection(ConnectableItem* from, ConnectableItem* to);
          void addStartToConnection(ConnectableItem* from);
          void addEndToConnection(ConnectableItem* to);
          const bool isConnectionLookingForStart() const;
          const bool isConnectionLookingForEnd() const;
          const bool isConnectionLooking() const;
          
        protected:
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
          virtual void keyPressEvent(QKeyEvent* event);
          
        private:
          QPointF _mousePosition;
          Connection* _newConnection;
      };
    }
  }
}

#endif
