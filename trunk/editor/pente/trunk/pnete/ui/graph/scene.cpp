// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/scene.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/item.hpp>

#include <pnete/ui/graph/style/raster.hpp>

#include <pnete/ui/util/action.hpp>

#include <pnete/data/internal.hpp>

#include <util/graphviz.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace scene
        {
          type::type ( ::xml::parse::type::net_type & net
                     , data::change_manager_t& change_manager
                     , QObject* parent
                     )
            : QGraphicsScene (parent)
            , _pending_connection (NULL)
            , _mouse_position (QPointF (0.0, 0.0))
            , _menu_context()
            , _net (net)
            , _change_manager (change_manager)
          {
            init_menu_context();

            connect ( &_change_manager
                    , SIGNAL ( signal_delete_transition
                               ( const QObject*
                               , const ::xml::parse::type::transition_type&
                               , ::xml::parse::type::net_type&
                               )
                             )
                    , SLOT ( slot_delete_transition
                             ( const QObject*
                             , const ::xml::parse::type::transition_type&
                             , ::xml::parse::type::net_type&
                             )
                           )
                    , Qt::DirectConnection
                    );
          }

          //! \todo This is duplicate code, also available in main window.
          void type::init_menu_context ()
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
            _menu_context.addSeparator();

            QAction* auto_layout_action
              (_menu_context.addAction (tr ("auto_layout")));
            connect ( auto_layout_action
                    , SIGNAL (triggered())
                    , SLOT (auto_layout())
                    );
          }

          void type::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
          {
            if (item* i = qgraphicsitem_cast<item*> (itemAt (event->scenePos())))
              {
                switch (i->type())
                  {
                  case item::connection_graph_type:
                    break;
                  case item::port_graph_type:
                  case item::top_level_port_graph_type:
                    {
                      QMenu menu;

                      QAction* action_set_type
                        (menu.addAction(tr("Set type")));
                      //                      connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

                      menu.addSeparator();

                      QAction* action_delete (menu.addAction(tr("Delete")));
                      //             connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));

                      menu.exec(event->screenPos());
                      event->accept();
                    }
                    break;
                  case item::transition_graph_type:
                    {
                      QMenu menu;

                      QAction* action_add_port (menu.addAction(tr("Add Port")));

                      menu.addSeparator();

                      QAction* action_delete (menu.addAction (tr("Delete")));


                      QAction* triggered (menu.exec(event->screenPos()));

                      if (triggered == action_delete)
                        {
                          slot_delete_transition (i);
                        }
                      else if (triggered == action_add_port)
                        {
                          std::cerr << "add port" << std::endl;
                        }
                      else if (!triggered)
                        {
                          //! \todo see QTBUG-21943
                          QPoint p ( event->widget()
                                   ->mapFromGlobal(event->screenPos())
                                   );
                          QMouseEvent mouseEvent( QEvent::MouseMove
                                                , p
                                                , Qt::NoButton
                                                , Qt::NoButton
                                                , event->modifiers()
                                                );
                          QApplication::sendEvent(event->widget(), &mouseEvent);
                        }

                      event->accept();
                    }
                    break;
                  case item::place_graph_type:
                    break;
                  default:
                    break;
                  }
              }
            else
              {
                _menu_context.popup(event->screenPos());
                event->accept();
              }
          }

          void type::slot_add_transition ()
          {
            qDebug() << "type::add_transition";
          }

          void type::slot_add_place ()
          {
            qDebug() << "type::add_place";
          }

          void type::slot_add_struct ()
          {
            qDebug() << "type::add_struct";
          }

          const QPointF& type::mouse_position() const
          {
            return _mouse_position;
          }

          connection::item* type::create_connection (bool only_reading)
          {
            connection::item * c (new connection::item (only_reading));
            addItem (c);
            c->setPos (QPointF (0.0, 0.0));
            return c;
          }

          void type::create_connection (connectable::item* item)
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

          void type::create_connection ( connectable::item* from
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

          void type::remove_pending_connection()
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

          void type::mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent)
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

          void type::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
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

          void type::keyPressEvent (QKeyEvent* event)
          {
            if (_pending_connection && event->key() == Qt::Key_Escape)
              {
                remove_pending_connection();
                event->accept();
                return;
              }

            QGraphicsScene::keyPressEvent (event);
          }

          void type::auto_layout()
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

          void type::slot_delete_transition
          ( const QObject* origin
          , const ::xml::parse::type::transition_type& trans
          , ::xml::parse::type::net_type& net
          )
          {
            if (origin != this && is_my_net (net))
              {
                foreach (QGraphicsItem* child, items())
                  {
                    if ( transition::item* transition_item
                       = qgraphicsitem_cast<transition::item*> (child)
                       )
                      {
                        if (&trans == &transition_item->transition())
                          {
                            remove_transition_item (transition_item);
                            transition_item->deleteLater();
                          }
                      }
                  }
              }
          }
          void type::remove_transition_item (transition::item* transition_item)
          {
            foreach (QGraphicsItem* child, transition_item->childItems())
              {
                if (port::item* port = qgraphicsitem_cast<port::item*> (child))
                  {
                    port->erase_connections (this);
                  }
              }

            removeItem (transition_item);
          }
          void type::slot_delete_transition (graph::item* graph_item)
          {
            transition::item* transition_item
              (qgraphicsitem_cast<transition::item*> (graph_item));

            if (!transition_item)
              {
                throw std::runtime_error
                  ("STRANGE: delete_transition for something else!?");
              }

            remove_transition_item (transition_item);

            _change_manager.delete_transition ( this
                                              , transition_item->transition()
                                              , transition_item->net()
                                              );

            transition_item->deleteLater();
          }

          bool type::is_my_net (const ::xml::parse::type::net_type& net)
          {
            return &net == &_net;
          }
        }
      }
    }
  }
}
