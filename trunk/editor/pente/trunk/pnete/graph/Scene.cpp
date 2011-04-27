#include "Scene.hpp"
#include "Connection.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Scene::Scene(QObject* parent)
      : QGraphicsScene(parent), _newConnection(NULL)
      {
        _mousePosition = QPointF(0.0f, 0.0f);
      }
      
      const QPointF& Scene::mousePosition() const
      {
        return _mousePosition;
      }
      
      const Connection* Scene::newConnection() const
      {
        return _newConnection;
      }
      
      void Scene::setNewConnection(Connection* newConnection)
      {
        _newConnection = newConnection;
        
        if(_newConnection)
        {
          addItem(_newConnection);
        }
      }
      
      void Scene::addConnection(ConnectableItem* from, ConnectableItem* to)
      {
        setNewConnection(new Connection(from, to));
      }
      void Scene::addStartToConnection(ConnectableItem* from)
      {
        if(!_newConnection)
        {
          return;
        }
        
        _newConnection->setStart(from);
        
        if(_newConnection->start() && _newConnection->end())
        {
          _newConnection = NULL;
        }
      }
      void Scene::addEndToConnection(ConnectableItem* to)
      {
        if(!_newConnection)
        {
          return;
        }
        
        _newConnection->setEnd(to);
        
        if(_newConnection->start() && _newConnection->end())
        {
          _newConnection = NULL;
        }
      }
      const bool Scene::isConnectionLookingForStart() const
      {
        return _newConnection && !_newConnection->start();
      }
      const bool Scene::isConnectionLookingForEnd() const
      {
        return _newConnection && !_newConnection->end();
      }
      const bool Scene::isConnectionLooking() const
      {
        return _newConnection && (!_newConnection->start() || !_newConnection->end());
      }
      
      void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
      {
        _mousePosition = mouseEvent->scenePos();
        if(_newConnection)
        {
          update();
        }
        
        QGraphicsScene::mouseMoveEvent(mouseEvent);
      }
      
      void Scene::removePendingConnection()
      {
        _newConnection->setEnd(NULL);
        _newConnection->setStart(NULL);
        //! \todo which one?
        delete _newConnection;
        //removeItem(_newConnection);
        _newConnection = NULL;
        update();
      }
      
      void Scene::keyPressEvent(QKeyEvent* event)
      {
        if(_newConnection && event->key() == Qt::Key_Escape)
        {
          removePendingConnection();
        }
      }
    }
  }
}
