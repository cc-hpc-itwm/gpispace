// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>

#include <pnete/data/internal.hpp>
#include <pnete/data/manager.hpp>

#include <pnete/weaver/weaver.hpp>

#include <pnete/util.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include <gvc.h>

#include <stack>
#include <stdexcept>

#include <iostream>

#include <boost/variant.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        scene::scene (net_type & net, QObject* parent)
          : QGraphicsScene (parent)
          , _pendingConnection (NULL)
          , _mousePosition (QPointF (0.0, 0.0))
          , _menu_context()
          , _net (net)
        {
          init_menu_context();
        }

        //! \todo This is duplicate code, also available in main window.
        void scene::init_menu_context ()
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

        void scene::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
        {
          QGraphicsScene::contextMenuEvent (event);

          if (!event->isAccepted())
          {
            _menu_context.popup(event->screenPos());
            event->accept();
          }
        }

        void scene::slot_add_transition ()
        {
          qDebug() << "scene::add_transition";
        }

        void scene::slot_add_place ()
        {
          qDebug() << "scene::add_place";
        }

        void scene::slot_add_struct ()
        {
          qDebug() << "scene::add_struct";
        }

        const QPointF& scene::mousePosition() const
        {
          return _mousePosition;
        }

        void scene::setPendingConnection (connection* connection)
        {
          removePendingConnection();
          _pendingConnection = connection;
//         if(_pendingConnection->scene() != this)
//         {
          addItem(_pendingConnection);
          _pendingConnection->setPos(0.0, 0.0);
//         }
          update (_pendingConnection->boundingRect());
        }

        bool scene::createPendingConnectionWith (connectable_item* item)
        {
          connection* c (new connection());
          setPendingConnection (c);

          if (item->direction() == connectable_item::IN)
          {
            c->setEnd (item);
          }
          else if (item->direction() == connectable_item::OUT)
          {
            c->setStart (item);
          }
          else
          {
            c->setStart (item);
          }

          return true;
        }

        void scene::create_connection ( connectable_item* from
                                      , connectable_item* to
                                      , bool only_reading
                                      )
        {
          connection* c (new connection (only_reading));
          addItem (c);
          c->setPos (0.0, 0.0);
          c->setStart (from);
          c->setEnd (to);
          update (c->boundingRect());
        }

        void scene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
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

        void scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
        {
          if(_pendingConnection)
          {
            foreach(QGraphicsItem* itemBelow, items(event->scenePos()))
            {
              connectable_item* portBelow = qgraphicsitem_cast<connectable_item*>(itemBelow);
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

        void scene::removePendingConnection()
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

        const connection* scene::pendingConnection() const
        {
          return _pendingConnection;
        }

        bool scene::pendingConnectionCanConnectTo(connectable_item* item) const
        {
          //! \note ugly enough?
          return _pendingConnection
            && !(_pendingConnection->start() && _pendingConnection->end())
            && (  (_pendingConnection->start() && _pendingConnection->start()->canConnectTo(item))
               || (_pendingConnection->end() && _pendingConnection->end()->canConnectTo(item))
               );
        }

        void scene::pendingConnectionConnectTo(connectable_item* item)
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

        void scene::keyPressEvent(QKeyEvent* event)
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

        QString scene::name() const
        {
          return "<<a scene>>";
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
              : _internal (agopen (const_cast<char*> ("___G"), AGDIGRAPH))
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
                gvLayout (_internal, g._internal, const_cast<char*> ("dot"));
                gvRender (_internal, g._internal, const_cast<char*> ("dot"), stderr);
                gvRender (_internal, g._internal, const_cast<char*> ("ps"), stdout);
                gvFreeLayout (_internal, g._internal);
              }
          };
        }

        void scene::auto_layout()
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
}
