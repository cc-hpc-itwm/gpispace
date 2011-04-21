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
        
        if(!_newConnection->isLookingForStart() && !_newConnection->isLookingForEnd())
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
        
        if(!_newConnection->isLookingForStart() && !_newConnection->isLookingForEnd())
        {
          _newConnection = NULL;
        }
      }
      const bool Scene::isConnectionLookingForStart() const
      {
        return _newConnection && _newConnection->isLookingForStart();
      }
      const bool Scene::isConnectionLookingForEnd() const
      {
        return _newConnection && _newConnection->isLookingForEnd();
      }
      const bool Scene::isConnectionLooking() const
      {
        return _newConnection && (_newConnection->isLookingForEnd() || _newConnection->isLookingForStart());
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
      
      void Scene::keyPressEvent(QKeyEvent* event)
      {
        if(_newConnection && event->key() == Qt::Key_Escape)
        {
          _newConnection->setEnd(NULL);
          _newConnection->setStart(NULL);
          delete _newConnection;
          //removeItem(_newConnection);
          _newConnection = NULL;
          update();
        }
      }
    }
  }
}
