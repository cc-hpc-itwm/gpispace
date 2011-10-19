// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/scene.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QDebug>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/item.hpp>

#include <pnete/ui/graph/style/raster.hpp>

#include <util/graphviz.hpp>

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

          QAction* auto_layout_action (new QAction (tr ("auto_layout"), this));
          connect ( auto_layout_action
                  , SIGNAL (triggered())
                  , SLOT (auto_layout())
                  );

          _menu_context.addMenu (menu_new);
          _menu_context.addAction (auto_layout_action);
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

        connection::item* scene::create_connection (bool only_reading)
        {
          connection::item * c (new connection::item (only_reading));
          addItem (c);
          c->setPos (0.0, 0.0);
          return c;
        }

        void scene::create_connection (connectable::item* item)
        {
          if (_pending_connection)
          {
            throw std::runtime_error ( "connection created while a different "
                                       "connection is still pending. Oo"
                                     );
          }

          _pending_connection = create_connection();
          if (item->direction() == connectable::direction::IN)
          {
            _pending_connection->end (item);
          }
          else
          {
            _pending_connection->start (item);
          }

          update (_pending_connection->boundingRect());
        }

        void scene::create_connection ( connectable::item* from
                                      , connectable::item* to
                                      , bool only_reading
                                      )
        {
          if (!to->is_connectable_with (from))
          {
            throw std::runtime_error
              ("tried hard-connecting non-connectable items.");
          }
          connection::item* c (create_connection (only_reading));
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
            // update();
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
            //! \note No, just casting to connectable::item* does NOT work. Qt!
            port::item* as_port (qgraphicsitem_cast<port::item*> (item));
            place::item* as_place (qgraphicsitem_cast<place::item*> (item));

            connectable::item* ci (as_port);
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
              if (qobject_cast<port::item*> (ci) && as_port)
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

        void scene::auto_layout()
        {
          typedef QHash<item*, graphviz::node_type> nodes_type;
          nodes_type nodes;

          graphviz::context_type context;
          graphviz::graph_type graph (context);
          graph.rankdir ("LR");
          graph.splines ("ortho");

          foreach (QGraphicsItem* i, items())
          {
            if ( ( i->type() == item::port_graph_type
                || i->type() == item::transition_graph_type
                || i->type() == item::place_graph_type
                || i->type() == item::top_level_port_graph_type
                 )
              && i->parentItem() == NULL)
            {
              nodes.insert (qgraphicsitem_cast<item*> (i), graph.add_node (i));
            }
          }
          foreach (QGraphicsItem* i, items())
          {
            if (connection::item* c = qgraphicsitem_cast<connection::item*> (i))
            {
              QGraphicsItem* start (c->start());
              QGraphicsItem* end (c->end());

              start = start->parentItem() ? start->parentItem() : start;
              end = end->parentItem() ? end->parentItem() : end;

              nodes_type::iterator start_node
                (nodes.find (qgraphicsitem_cast<item*> (start)));
              nodes_type::iterator end_node
                (nodes.find (qgraphicsitem_cast<item*> (end)));

              graph.add_edge (*start_node, *end_node);
            }
          }

          graph.layout ("dot");

          for ( nodes_type::const_iterator it (nodes.begin()), end (nodes.end())
              ; it != end
              ; ++it
              )
          {
            it.key()->setPos (style::raster::snap (it.value().position()));
          }
        }
      }
    }
  }
}
