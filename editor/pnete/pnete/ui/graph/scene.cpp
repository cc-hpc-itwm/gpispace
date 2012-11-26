// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/scene.hpp>

#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/transition.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/pending_connection.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/util/action.hpp>
#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <util/graphviz.hpp>
#include <util/qt/cast.hpp>

#include <list>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        scene_type::scene_type ( const data::handle::net& net
                               , data::internal_type* internal
                               , QObject* parent
                               )
          : QGraphicsScene (parent)
          , _pending_connection (NULL)
          , _mouse_position (QPointF (0.0, 0.0))
          , _menu_context()
          , _net (net)
          , _internal (internal)
        {
          init_menu_context();

          // transition
          _net.connect_to_change_mgr
            ( this
            , "transition_added"
            , "const data::handle::transition&"
            );
          _net.connect_to_change_mgr
            ( this
            , "transition_deleted"
            , "const data::handle::transition&"
            );

          // place
          _net.connect_to_change_mgr
            ( this
            , "place_added"
            , "const data::handle::place&"
            );
          _net.connect_to_change_mgr
            ( this
            , "place_deleted"
            , "const data::handle::place&"
            );

          // connections
          //! \note This is not really in responsibility of the net,
          // but we would need to connect every transition being added
          // to the signal/slot again, which would be a lot of
          // reconnection happening, every time when adding or
          // removing transitions. Thus we just use the net's change
          // manager (as that should be the same as the transitions'
          // one) and hook for any connection changing in any
          // transition.
          _net.connect_to_change_mgr
            ( this
            , "connection_added"
            , "const data::handle::connect&, "
              "const data::handle::place&, const data::handle::port&"
            );
          _net.connect_to_change_mgr
            ( this
            , "connection_added"
            , "const data::handle::connect&, "
              "const data::handle::port&, const data::handle::place&"
            );
        }

        //! \todo This is duplicate code, also available in main window.
        void scene_type::init_menu_context ()
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

        void scene_type::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
        {
          if (base_item* i = qgraphicsitem_cast<base_item*> (itemAt (event->scenePos())))
          {
            switch (i->type())
            {
            case base_item::connection_graph_type:
              break;
            case base_item::port_graph_type:
            case base_item::top_level_port_graph_type:
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
            case base_item::transition_graph_type:
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
            case base_item::place_graph_type:
            {
              QMenu menu;

              QAction* action_delete (menu.addAction (tr("Delete")));

              QAction* triggered (menu.exec(event->screenPos()));

              if (triggered == action_delete)
              {
                slot_delete_place (i);
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

        data::internal_type* scene_type::internal() const
        {
          return _internal;
        }

        data::change_manager_t& scene_type::change_manager() const
        {
          return internal()->change_manager();
        }

        void scene_type::slot_add_struct ()
        {
          qDebug() << "scene_type::add_struct";
        }

        const QPointF& scene_type::mouse_position() const
        {
          return _mouse_position;
        }

        connection_item* scene_type::create_connection (bool only_reading)
        {
          connection_item * c (new connection_item (boost::none, only_reading));
          addItem (c);
          c->set_just_pos_but_not_in_property (0.0, 0.0);
          return c;
        }

        void scene_type::create_connection (connectable_item* item)
        {
          if (_pending_connection)
          {
            throw std::runtime_error ( "connection created while a different "
                                     "connection is still pending. Oo"
                                     );
          }

          _pending_connection = new pending_connection (item, _mouse_position);
          _pending_connection->set_just_pos_but_not_in_property (0.0, 0.0);
          addItem (_pending_connection);

          update (_pending_connection->boundingRect());
        }

        void scene_type::create_connection ( connectable_item* from
                                     , connectable_item* to
                                     , bool only_reading
                                     )
        {
          if (!to->is_connectable_with (from))
          {
            throw std::runtime_error
              ("tried hard-connecting non-connectable items.");
          }
          connection_item* c (create_connection (only_reading));
          c->start (from);
          c->end (to);
          update (c->boundingRect());
        }

        void scene_type::remove_pending_connection()
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

        void scene_type::mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent)
        {
          if (_pending_connection)
          {
            update (_pending_connection->boundingRect());
            _pending_connection->open_end
              (_mouse_position = mouseEvent->scenePos());
            update (_pending_connection->boundingRect());
          }
          else
          {
            _mouse_position = mouseEvent->scenePos();
          }

          QGraphicsScene::mouseMoveEvent (mouseEvent);
        }

        void scene_type::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!_pending_connection)
          {
            QGraphicsScene::mouseReleaseEvent (event);
            return;
          }

          foreach (const QGraphicsItem* item, items (event->scenePos()))
          {
            const port_item* as_port
              (qgraphicsitem_cast<const port_item*> (item));
            const place_item* as_place
              (qgraphicsitem_cast<const place_item*> (item));

            const connectable_item* ci
              ( as_port
              ? static_cast<const connectable_item*> (as_port)
              : static_cast<const connectable_item*> (as_place)
              );
            if (!ci)
            {
              continue;
            }

            const connectable_item* pending (_pending_connection->fixed_end());

            if (ci->direction() == pending->direction())
            {
              throw std::runtime_error
                ("connecting two items with same direction");
            }

            const port_item* pending_as_port
              (qgraphicsitem_cast<const port_item*> (pending));
            const place_item* pending_as_place
              (qgraphicsitem_cast<const place_item*> (pending));

            if (as_port && pending_as_port)
            {
              if (as_port->direction() == connectable::direction::IN)
              {
                change_manager().add_connection
                  (this, pending_as_port->handle(), as_port->handle());
              }
              else
              {
                change_manager().add_connection
                  (this, as_port->handle(), pending_as_port->handle());
              }
            }
            else
            {
              const port_item* port (as_port ? as_port : pending_as_port);
              const place_item* place (as_place ? as_place : pending_as_place);

              if (port->direction() == connectable::direction::IN)
              {
                change_manager().add_connection
                  (this, place->handle(), port->handle());
              }
              else
              {
                change_manager().add_connection
                  (this, port->handle(), place->handle());
              }
            }

            event->accept();
            break;
          }

          remove_pending_connection();

          QGraphicsScene::mouseReleaseEvent (event);
        }

        void scene_type::keyPressEvent (QKeyEvent* event)
        {
          if (_pending_connection && event->key() == Qt::Key_Escape)
          {
            remove_pending_connection();
            event->accept();
            return;
          }

          QGraphicsScene::keyPressEvent (event);
        }

        void scene_type::auto_layout()
        {
          typedef boost::unordered_map< base_item*
                                      , graphviz::node_type
                                      > nodes_map_type;
          nodes_map_type nodes;

          graphviz::context_type context;
          graphviz::graph_type graph (context);
          graph.rankdir ("LR");
          graph.splines ("ortho");

          foreach (QGraphicsItem* i, items())
          {
            if ( (  i->type() == base_item::port_graph_type
                 || i->type() == base_item::transition_graph_type
                 || i->type() == base_item::place_graph_type
                 || i->type() == base_item::top_level_port_graph_type
                 )
               && i->parentItem() == NULL
               )
            {
              nodes.insert ( nodes_map_type::value_type
                             ( qgraphicsitem_cast<base_item*> (i)
                             , graph.add_node (i)
                             )
                           );
            }
          }

          typedef boost::unordered_map< connection_item*
                                        , graphviz::edge_type
                                        > edges_map_type;
          edges_map_type edges;

          foreach (QGraphicsItem* i, items())
          {
            if ( connection_item* c
               = qgraphicsitem_cast<connection_item*> (i)
               )
            {
              QGraphicsItem* start (c->start());
              QGraphicsItem* end (c->end());

              start = start->parentItem() ? start->parentItem() : start;
              end = end->parentItem() ? end->parentItem() : end;

              nodes_map_type::iterator start_node
                (nodes.find (qgraphicsitem_cast<base_item*> (start)));
              nodes_map_type::iterator end_node
                (nodes.find (qgraphicsitem_cast<base_item*> (end)));

              if (start_node != nodes.end() && end_node != nodes.end())
              {
                edges.insert
                  ( edges_map_type::value_type
                  ( c
                  , graph.add_edge ( start_node->second
                                   , end_node->second
                                   )
                  )
                  );
              }
            }
          }

          graph.layout ("dot");

          BOOST_FOREACH (nodes_map_type::value_type& it, nodes)
          {
            it.first->setPos (style::raster::snap (it.second.position()));
          }

          BOOST_FOREACH (const edges_map_type::value_type& edge, edges)
          {
            //! \todo enable this, before repair connection::shape
            //                edge.first->fixed_points (edge.second.points());
          }
        }

        void scene_type::remove_transition_item (transition_item* transition_item)
        {
          foreach (QGraphicsItem* child, transition_item->childItems())
          {
            if (port_item* port = qgraphicsitem_cast<port_item*> (child))
            {
              port->erase_connections (this);
            }
          }

          removeItem (transition_item);
        }

        template<typename item_type, typename handle_type>
          item_type* scene_type::item_with_handle (const handle_type& handle)
        {
          foreach (QGraphicsItem* child, items())
          {
            if (item_type* item = qgraphicsitem_cast<item_type*> (child))
            {
              if (item->handle() == handle)
              {
                return item;
              }
            }
          }
          return NULL;
        }

        template<typename handle_type>
          bool scene_type::is_in_my_net (const handle_type& handle)
        {
          return handle.get().parent()->id() == net().id();
        }

        const data::handle::net& scene_type::net() const
        {
          return _net;
        }

        // ## trigger modification ###################################
        // # transition ##############################################
        void scene_type::slot_add_transition ()
        {
          change_manager().add_transition (this, net());
        }

        void scene_type::slot_delete_transition (base_item* graph_item)
        {
          transition_item* transition_item
            (fhg::util::qt::throwing_qgraphicsitem_cast<transition_item*> (graph_item));

          remove_transition_item (transition_item);

          change_manager().delete_transition ( this
                                             , transition_item->handle()
                                             );

          transition_item->deleteLater();
        }

        // # place ###################################################
        void scene_type::slot_add_place ()
        {
          change_manager().add_place (this, net());
        }

        void scene_type::slot_delete_place (base_item* graph_item)
        {
          place_item* place_item
            (fhg::util::qt::throwing_qgraphicsitem_cast<place_item*> (graph_item));

          place_item->erase_connections (this);
          removeItem (place_item);

          place_item->handle().remove (this);

          place_item->deleteLater();
        }

        // ## react on modification ##################################

        // # connection ##############################################
        void scene_type::connection_added
          ( const QObject* origin
          , const data::handle::connect& connection
          , const data::handle::place& from
          , const data::handle::port& to
          )
        {
          if (is_in_my_net (from))
          {
            //! \todo Weaver.
            create_connection ( item_with_handle<place_item> (from)
                              , item_with_handle<port_item> (to)
                              , false
                              );
          }
        }
        void scene_type::connection_added
          ( const QObject* origin
          , const data::handle::connect& connection
          , const data::handle::port& from
          , const data::handle::place& to
          )
        {
          if (is_in_my_net (to))
          {
            //! \todo Weaver.
            create_connection ( item_with_handle<port_item> (from)
                              , item_with_handle<place_item> (to)
                              , false
                              );
          }
        }

        // # transition ##############################################
        void scene_type::transition_added
          (const QObject* origin, const data::handle::transition& transition)
        {
          if (is_in_my_net (transition))
          {
            transition_item* item (new transition_item (transition));

            addItem (item);

            weaver::item_by_name_type place_by_name;

            weaver::transition wt
              ( _internal
              , this
              , item
              , transition.parent().get().make_reference_id()
              , place_by_name
              );
            weaver::from::transition (&wt, transition.id());

            if (origin == this)
            {
              item->no_undo_setPos (mouse_position());
              item->repositionChildrenAndResize();
            }
          }
        }

        void scene_type::transition_deleted
          (const QObject* origin, const data::handle::transition& transition)
        {
          if (origin != this && is_in_my_net (transition))
          {
            transition_item* item
              (item_with_handle<transition_item> (transition));
            remove_transition_item (item);
            item->deleteLater();
          }
        }

        // # place ###################################################
        void scene_type::place_added
          (const QObject* origin, const data::handle::place& place)
        {
          if (is_in_my_net (place))
          {
            place_item* item (new place_item (place));

            addItem (item);

            weaver::item_by_name_type place_by_name;

            weaver::place wp (item, place_by_name);
            weaver::from::place (&wp, place.id());

            if (origin == this)
            {
              item->no_undo_setPos (mouse_position());
            }
          }
        }

        void scene_type::place_deleted
          (const QObject* origin, const data::handle::place& place)
        {
          if (origin != this && is_in_my_net (place))
          {
            place_item* item (item_with_handle<place_item> (place));
            removeItem (item);
            item->deleteLater();
          }
        }
      }
    }
  }
}
