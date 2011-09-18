
#include <pnete/ui/GraphConnectableItem.hpp>
#include <pnete/ui/GraphConnection.hpp>
#include <pnete/ui/GraphScene.hpp>

#include <pnete/data/internal.hpp>
#include <pnete/data/manager.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include <gvc.h>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Scene::Scene (data::internal::ptr data, QObject* parent)
      : QGraphicsScene (parent)
      , _pendingConnection (NULL)
      , _mousePosition (QPointF (0.0, 0.0))
      , _menu_context()
      , _data (data)
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
        update (_pendingConnection->boundingRect());
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
        QRectF old_area ( _pendingConnection
                        ? _pendingConnection->boundingRect()
                        : QRectF()
                        );
        _mousePosition = mouseEvent->scenePos();
        if(_pendingConnection)
        {
          update (old_area);
          update (QRectF (QPointF (0.0, 0.0), _mousePosition));
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
              //! \todo No idea what to update here? The old connection should be updated in removePendingConnection(). 
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
          QRectF area (_pendingConnection->boundingRect());
          _pendingConnection->setEnd(NULL);
          _pendingConnection->setStart(NULL);
          //! \todo which one?
          delete _pendingConnection;
          //removeItem(_pendingConnection);
          _pendingConnection = NULL;
          update (area);
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

      void Scene::save (const QString& filename) const
      {
        data::manager::instance().save (_data, filename);
      }

      QString Scene::name() const
      {
        return _data->name();
      }

      namespace GraphViz
      {
        class Graph
        {
          private:
            Agraph_t* _internal;
            std::vector<Agnode_t*> _nodes;
            std::vector<Agedge_t*> _edges;
          public:
            typedef std::vector<Agnode_t*>::size_type node_type;
            typedef std::vector<Agedge_t*>::size_type edge_type;

            Graph()
            : _internal (agopen ("___G", AGDIGRAPH))
            {
            }

            node_type add_node (const std::string& name)
            {
              _nodes.push_back (agnode (_internal, const_cast<char*> (name.c_str())));
              return _nodes.size() - 1;
            }
            edge_type add_edge (node_type from, node_type to)
            {
              _edges.push_back (agedge (_internal, _nodes[from], _nodes[to]));
              return _edges.size() - 1;
            }

            ~Graph()
            {
              agclose (_internal);
            }

            friend class Context;
        };
        class Context
        {
          private:
            GVC_t* _internal;
          public:
            Context ()
            : _internal (gvContext())
            {
            }

            ~Context()
            {
              gvFreeContext (_internal);
            }

            void layout (Graph& g)
            {
              gvLayout (_internal, g._internal, "dot");
              gvRender (_internal, g._internal, "dot", stderr);
              gvRender (_internal, g._internal, "ps", stdout);
              gvFreeLayout (_internal, g._internal);
            }
        };
      }

      void Scene::auto_layout()
      {
        //! \todo Init graphviz graph and context.
        //! \todo Iterate over all items, calling add_to_graphviz_graph on all.
        //! \todo Tell graphviz to autolayout.
        //! \todo Iterate over graphviz data and relayout graph.
        //! \todo Free graphviz stuff.

        GraphViz::Context gv_context;
        GraphViz::Graph g;

        GraphViz::Graph::node_type a (g.add_node ("a"));
        GraphViz::Graph::node_type b (g.add_node ("b"));
        GraphViz::Graph::node_type c (g.add_node ("c"));

        GraphViz::Graph::edge_type edge_ab (g.add_edge (a, b));
        GraphViz::Graph::edge_type edge_bc (g.add_edge (b, c));
        GraphViz::Graph::edge_type edge_ca (g.add_edge (c, a));

        gv_context.layout (g);
      }
    }
  }
}
