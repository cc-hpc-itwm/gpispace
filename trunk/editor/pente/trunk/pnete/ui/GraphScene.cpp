#include "GraphScene.hpp"
#include "GraphConnection.hpp"
#include "GraphConnectableItem.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Scene::Scene(QObject* parent)
        : QGraphicsScene(parent)
        , _pendingConnection(NULL)
        , _mousePosition(QPointF(0.0, 0.0))
        , _menu_new ("New")
        , _menu_context ()
      {
        init_menu_new();
        init_menu_context();
      }

      Scene::Scene(const QRectF& sceneRect, QObject* parent)
        : QGraphicsScene(sceneRect, parent)
        , _pendingConnection(NULL)
        , _mousePosition(QPointF(0.0, 0.0))
        , _menu_new ("New")
        , _menu_context()
      {
        init_menu_new();
        init_menu_context();
      }

      void Scene::init_menu_new ()
      {
        QAction* action_add_transition (_menu_new.addAction (tr("transition")));
        connect ( action_add_transition
                , SIGNAL(triggered())
                , this
                , SLOT(slot_add_transition())
                );

        QAction* action_add_place (_menu_new.addAction (tr("place")));
        connect ( action_add_place
                , SIGNAL(triggered())
                , this
                , SLOT(slot_add_place())
                );

        _menu_new.addSeparator();

        QAction* action_add_struct (_menu_new.addAction (tr("struct")));
        connect ( action_add_struct
                , SIGNAL(triggered())
                , this
                , SLOT(slot_add_struct())
                );
      }

      void Scene::init_menu_context()
      {
        _menu_context.addMenu (&_menu_new);
      }

      void Scene::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
      {
        QGraphicsScene::contextMenuEvent (event);

        if (!event->isAccepted())
        {
          _menu_context.popup(event->screenPos());
          event->accept();
        }
      }

      void Scene::slot_add_transition ()
      {
        qDebug() << "Scene::add_transition";
      }

      void Scene::slot_add_place ()
      {
        qDebug() << "Scene::add_place";
      }

      void Scene::slot_add_struct ()
      {
        qDebug() << "Scene::add_struct";
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

      QMenu * Scene::menu_new ()
      {
        return &_menu_new;
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
          foreach(QGraphicsItem* itemBelow, items(event->scenePos()))
          {
            ConnectableItem* portBelow = qgraphicsitem_cast<ConnectableItem*>(itemBelow);
            if(portBelow && pendingConnectionCanConnectTo(portBelow))
            {
              pendingConnectionConnectTo(portBelow);
              event->accept();
              update();
              return;
            }
          }

          removePendingConnection();
        }

        event->ignore();

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
          event->accept();
        }
        else
        {
          event->ignore();
        }
      }
    }
  }
}
