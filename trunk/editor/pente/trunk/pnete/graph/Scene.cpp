#include "Scene.hpp"
#include "Connection.hpp"
#include "ConnectableItem.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRectF>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Scene::Scene(QObject* parent)
      : QGraphicsScene(parent),
      _pendingConnection(NULL),
      _mousePosition(QPointF(0.0, 0.0))
      {
      }
      
      Scene::Scene(const QRectF& sceneRect, QObject* parent)
      : QGraphicsScene(sceneRect, parent),
      _pendingConnection(NULL),
      _mousePosition(QPointF(0.0, 0.0))
      {
      }
      
      const QPointF& Scene::mousePosition() const
      {
        return _mousePosition;
      }
      
      void Scene::setPendingConnection(Connection* connection)
      {
        removePendingConnection();
        _pendingConnection = connection;
        if(_pendingConnection->scene() != this)
        {
          addItem(_pendingConnection);
          _pendingConnection->setPos(0.0, 0.0);
        }
        update();
      }
      
      bool Scene::createPendingConnectionWith(ConnectableItem* item)
      {
        if(item->canConnectIn(ConnectableItem::ANYDIRECTION))
        {
          ;//! \note Aaaaaargh.
        }
        else if(item->canConnectIn(ConnectableItem::IN))
        {
          setPendingConnection(new Connection(NULL, item));
          return true;
        }
        else if(item->canConnectIn(ConnectableItem::OUT))
        {
          setPendingConnection(new Connection(item, NULL));
          return true;
        }
        return false;
      }
      
      void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
      {
        _mousePosition = mouseEvent->scenePos();
        if(_pendingConnection)
        {
          update();
        }
        
        QGraphicsScene::mouseMoveEvent(mouseEvent);
      }
      
      void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
      {
        if(_pendingConnection)
        {
          QList<QGraphicsItem*> itemsBelow = items(event->scenePos());
          for(QList<QGraphicsItem*>::iterator it = itemsBelow.begin(); it != itemsBelow.end(); ++it)
          {
            ConnectableItem* portBelow = qgraphicsitem_cast<ConnectableItem*>(*it);
            if(portBelow && pendingConnectionCanConnectTo(portBelow))
            {
              pendingConnectionConnectTo(portBelow);
              event->setAccepted(true);
              update();
              return;
            }
          }
          
          removePendingConnection();
        }
        
        QGraphicsScene::mouseReleaseEvent(event);
      }
      
      void Scene::removePendingConnection()
      {
        if(_pendingConnection)
        {
          _pendingConnection->setEnd(NULL);
          _pendingConnection->setStart(NULL);
          //! \todo which one?
          delete _pendingConnection;
          //removeItem(_pendingConnection);
          _pendingConnection = NULL;
          update();
        }
      }
      
      const Connection* Scene::pendingConnection() const
      {
        return _pendingConnection;
      }
      
      bool Scene::pendingConnectionCanConnectTo(ConnectableItem* item) const
      {
        //! \note ugly enough?
        return _pendingConnection
               && !(_pendingConnection->start() && _pendingConnection->end())
               && (  (_pendingConnection->start() && _pendingConnection->start()->canConnectTo(item))
                  || (_pendingConnection->end() && _pendingConnection->end()->canConnectTo(item))
                  );
      }
      
      void Scene::pendingConnectionConnectTo(ConnectableItem* item)
      {
        if(pendingConnectionCanConnectTo(item))
        {
          if(_pendingConnection->start())
          {
            _pendingConnection->setEnd(item);
          }
          else
          {
            _pendingConnection->setStart(item);
          }
          _pendingConnection = NULL;
        }
      }
      
      void Scene::keyPressEvent(QKeyEvent* event)
      {
        if(_pendingConnection && event->key() == Qt::Key_Escape)
        {
          removePendingConnection();
        }
      }
    }
  }
}
