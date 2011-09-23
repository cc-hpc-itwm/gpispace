// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>

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
          , _pending_connection (NULL)
          , _mouse_position (QPointF (0.0, 0.0))
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

        const QPointF& scene::mouse_position() const
        {
          return _mouse_position;
        }

        connection* scene::create_connection (bool only_reading)
        {
          connection* c (new connection(only_reading));
          addItem (c);
          c->setPos (0.0, 0.0);
          return c;
        }

        void scene::create_connection (connectable_item* item)
        {
          if (_pending_connection)
          {
            throw std::runtime_error ( "connection created while a different "
                                       "connection is still pending. Oo"
                                     );
          }

          _pending_connection = create_connection();
          if (item->direction() == connectable_item::IN)
          {
            _pending_connection->end (item);
          }
          else
          {
            _pending_connection->start (item);
          }

          update (_pending_connection->boundingRect());
        }

        void scene::create_connection ( connectable_item* from
                                      , connectable_item* to
                                      , bool only_reading
                                      )
        {
          if (!to->is_connectable_with (from))
          {
            throw std::runtime_error
              ("tried hard-connecting non-connectable items.");
          }
          connection* c (create_connection (only_reading));
          c->start (from);
          c->end (to);
          update (c->boundingRect());
        }

        void scene::remove_pending_connection()
        {
          if (!_pending_connection)
          {
            return;
          }

          const QRectF area (_pending_connection->boundingRect());
          delete _pending_connection;
          _pending_connection = NULL;
          update(area);
        }

        void scene::mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent)
        {
          if (_pending_connection)
          {
            const QRectF old_area (_pending_connection->boundingRect());
            _mouse_position = mouseEvent->scenePos();
            update (old_area);

            update ( QRectF ( QPointF ( qMin (0.0, _mouse_position.x())
                                      , qMin (0.0, _mouse_position.y())
                                      )
                            , QPointF ( qMax (0.0, _mouse_position.x())
                                      , qMax (0.0, _mouse_position.y())
                                      )
                            )
                   );
          }
          else
          {
            _mouse_position = mouseEvent->scenePos();
            update();
          }

          QGraphicsScene::mouseMoveEvent (mouseEvent);
        }

        void scene::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!_pending_connection)
          {
            QGraphicsScene::mouseReleaseEvent (event);
            return;
          }

          foreach (QGraphicsItem* item, items(event->scenePos()))
          {
            //! \note No, just casting to connectable_item* does NOT work. Qt!
            port* as_port (qgraphicsitem_cast<port*> (item));
            place* as_place (qgraphicsitem_cast<place*> (item));

            connectable_item* ci (as_port);
            if (!ci)
            {
              ci = as_place;
            }
            if (!ci)
            {
              continue;
            }

            if (ci->is_connectable_with (_pending_connection->non_free_side()))
            {
              if (qobject_cast<port*> (ci) && as_port)
              {
                //! \todo insert place and connect with that place in between.
              }

              _pending_connection->free_side (ci);
              update (_pending_connection->boundingRect());
              _pending_connection = NULL;
              event->accept();
              break;
            }
          }

          remove_pending_connection();

          QGraphicsScene::mouseReleaseEvent (event);
        }

        void scene::keyPressEvent (QKeyEvent* event)
        {
          if (_pending_connection && event->key() == Qt::Key_Escape)
          {
             remove_pending_connection();
             event->accept();
             return;
          }
          QGraphicsScene::keyPressEvent (event);
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
