#include "GraphScene.hpp"
#include "GraphConnection.hpp"
#include "GraphConnectableItem.hpp"
#include "helper/GraphTraverser.hpp"
#include "helper/TraverserReceiver.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Scene::Scene (QObject* parent)
      : QGraphicsScene (parent)
      , _pendingConnection (NULL)
      , _mousePosition (QPointF (0.0, 0.0))
      , _menu_context()
      , _name (tr ("(untitled)"))
      {
        init_menu_context();
      }

      Scene::Scene (const QString& filename, QObject* parent)
      : QGraphicsScene (parent)
      , _pendingConnection (NULL)
      , _mousePosition (QPointF (0.0, 0.0))
      , _menu_context()
      , _name (filename.section ('/', -1))
      {
        init_menu_context();

        //! \todo load data from file.
      }

      //! \todo This is duplicate code, also available in main window.
      void Scene::init_menu_context ()
      {
        //! \todo This QMenu most likely is leaked.
        QMenu* menu_new (new QMenu (tr ("new"), NULL));
        QAction* action_add_transition (menu_new->addAction (tr ("transition")));
        connect ( action_add_transition
                , SIGNAL (triggered())
                , SLOT (slot_add_transition())
                );

        QAction* action_add_place (menu_new->addAction (tr ("place")));
        connect ( action_add_place
                , SIGNAL (triggered())
                , SLOT (slot_add_place())
                );

        menu_new->addSeparator();

        QAction* action_add_struct (menu_new->addAction (tr ("struct")));
        connect ( action_add_struct
                , SIGNAL (triggered())
                , SLOT (slot_add_struct())
                );

        _menu_context.addMenu (menu_new);
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

      void Scene::save (const QString& filename)
      {
        _name = (filename.section ('/', -1));

        //! \todo dump _internal_data via we::*.
        QFile xmlFile (filename);

        if (!xmlFile.open (QIODevice::WriteOnly | QIODevice::Text))
        {
          return;
        }

        helper::GraphTraverser traverser (this);
        helper::TraverserReceiver receiver;

        QTextStream out (&xmlFile);
        out << traverser.traverse (&receiver, QFileInfo (xmlFile).baseName())
            << "\n";
        out.flush();

        xmlFile.close();
      }

      const QString& Scene::name() const
      {
        return _name;
      }
    }
  }
}
