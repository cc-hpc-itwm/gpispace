// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/scene.hpp>

#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/place_map.hpp>
#include <pnete/data/handle/transition.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/data/manager.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/editor_window.hpp>
#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/pending_connection.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/place_map.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/port_place_association.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/util/action.hpp>
#include <pnete/weaver/display.hpp>

#include <util/graphviz.hpp>
#include <util/qt/boost_connect.hpp>
#include <util/qt/cast.hpp>
#include <util/qt/parent.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/transition.hpp>

#include <list>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace
        {
          QAction* separator (QObject* parent)
          {
            QAction* sep (new QAction (parent));
            sep->setSeparator (true);
            return sep;
          }
        }

        QAction* scene_type::connect_action (QAction* action, const char* slot)
        {
          connect (action, SIGNAL (triggered()), slot);
          return action;
        }

        template<typename FUN_TYPE>
        QAction* scene_type::connect_action (QAction* action, FUN_TYPE fun)
        {
          fhg::util::qt::boost_connect<void()>
            (action, SIGNAL (triggered()), this, fun);
          return action;
        }

        namespace
        {
          void set_function_name ( const data::handle::function& handle
                                 , const QString& dialog_title
                                 , const QString& prompt
                                 , QObject* origin
                                 )
          {
            bool ok;
            const QString name
              ( QInputDialog::getText
                ( NULL
                , dialog_title
                , prompt
                , QLineEdit::Normal
                , QString::fromStdString (handle.get().name().get_value_or (""))
                , &ok
                )
              );
            if (ok)
            {
              handle.set_name (origin, name);
            }
          }
        }

        scene_type::scene_type ( const data::handle::net& net
                               , const data::handle::function& function
                               , QObject* parent
                               )
          : QGraphicsScene (parent)
          , _pending_connection (NULL)
          , _mouse_position (QPointF (0.0, 0.0))
          , _net (net)
          , _function (function)
          //! \todo Don't default to center of scene, but center of visible scene!
          , _add_transition_action
            ( connect_action ( new QAction (tr ("new_transition"), this)
                             , boost::bind ( &data::handle::net::add_transition
                                           , _net
                                           , this
                                           , boost::none
                                           )
                             )
            )
          , _add_place_action
            ( connect_action ( new QAction (tr ("new_place"), this)
                             , boost::bind ( &data::handle::net::add_place
                                           , _net
                                           , this
                                           , boost::none
                                           )
                             )
            )
          , _add_top_level_port_in_action
            ( connect_action ( new QAction (tr ("new_top_level_port_in"), this)
                             , boost::bind ( &data::handle::function::add_port
                                           , _function
                                           , this
                                           , we::type::PORT_IN
                                           , boost::none
                                           )
                             )
            )
          , _add_top_level_port_out_action
            ( connect_action ( new QAction (tr ("new_top_level_port_out"), this)
                             , boost::bind ( &data::handle::function::add_port
                                           , _function
                                           , this
                                           , we::type::PORT_OUT
                                           , boost::none
                                           )
                             )
            )
          , _add_top_level_port_tunnel_action
            ( connect_action ( new QAction (tr ("new_top_level_port_tunnel"), this)
                             , boost::bind ( &data::handle::function::add_port
                                           , _function
                                           , this
                                           , we::type::PORT_TUNNEL
                                           , boost::none
                                           )
                             )
            )
          , _auto_layout_action
            ( connect_action ( new QAction (tr ("auto_layout"), this)
                             , SLOT (auto_layout())
                             )
            )
          , _actions ( QList<QAction*>()
                     << _add_transition_action
                     << _add_place_action
                     << _add_top_level_port_in_action
                     << _add_top_level_port_out_action
                     << _add_top_level_port_tunnel_action
                     << separator (this)
                     << _auto_layout_action
                     << separator (this)
                     << connect_action
                       ( new QAction (tr ("set_function_name"), this)
                       , boost::bind ( set_function_name
                                     , _function
                                     , tr ("set_function_name_title")
                                     , tr ("set_function_name_prompt")
                                     , this
                                     )
                       )
                     )
        {
          // transition
          _net.connect_to_change_mgr
            (this, "transition_added", "data::handle::transition");
          _net.connect_to_change_mgr
            (this, "transition_deleted", "data::handle::transition");

          // place
          _net.connect_to_change_mgr
            (this, "place_added", "data::handle::place");
          _net.connect_to_change_mgr
            (this, "place_deleted", "data::handle::place");

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
            , "data::handle::connect, data::handle::place, data::handle::port"
            );
          _net.connect_to_change_mgr
            (this, "connection_removed", "data::handle::connect");

          _net.connect_to_change_mgr
            (this, "place_map_added", "data::handle::place_map");
          _net.connect_to_change_mgr
            (this, "place_map_removed", "data::handle::place_map");

          // top-level-ports
          _net.connect_to_change_mgr
            (this, "port_added", "data::handle::port");
          _net.connect_to_change_mgr
            (this, "port_deleted", "data::handle::port");

          _net.connect_to_change_mgr
            ( this
            , "place_association_set"
            , "data::handle::port, boost::optional<std::string>"
            );
        }

        QList<QAction*> scene_type::actions() const
        {
          return _actions;
        }

        namespace
        {
          bool can_rename ( const data::handle::place& handle
                          , const QString& name
                          )
          {
            return !handle.get().parent()->has_place (name.toStdString());
          }

          bool can_rename ( const data::handle::port& handle
                          , const QString& name
                          )
          {
            return handle.can_rename_to (name);
          }

          bool can_rename ( const data::handle::transition& handle
                          , const QString& name
                          )
          {
            return !handle.get().parent()->has_transition (name.toStdString());
          }

          template<typename handle_type>
          void set_name_for_handle ( const handle_type& handle
                                   , const QString& dialog_title
                                   , const QString& prompt
                                   , const QString& current_name
                                   , QWidget* widget
                                   , QObject* origin
                                   )
          {
            bool ok;
            const QString name
              ( QInputDialog::getText
                ( widget
                , dialog_title
                , prompt
                , QLineEdit::Normal
                , current_name
                , &ok
                )
              );
            if (ok)
            {
              if ( handle.get().name() == name.toStdString()
                 || can_rename (handle, name)
                 )
              {
                handle.set_name (origin, name);
              }
              else
              {
                const QMessageBox::StandardButton reply
                  ( QMessageBox::critical
                    ( widget
                    , QObject::tr ("name_already_exists_head")
                    , QObject::tr ("name_already_exists_msg")
                    , QMessageBox::Cancel | QMessageBox::Retry
                    )
                  );

                if (reply != QMessageBox::Cancel)
                {
                  set_name_for_handle ( handle
                                      , dialog_title
                                      , prompt
                                      , name
                                      , widget
                                      , origin
                                      );
                }
              }
            }
          }

          template<typename handle_type>
          void set_we_type_for_handle ( const handle_type& handle
                                      , const QString& dialog_title
                                      , const QString& prompt
                                      , const QString& current_type
                                      , QWidget* widget
                                      , QObject* origin
                                      )
          {
            bool ok;
            const QString text
              ( QInputDialog::getText
                ( widget
                , dialog_title
                , prompt
                , QLineEdit::Normal
                , current_type
                , &ok
                )
              );
            if (ok && !text.isEmpty())
            {
              handle.set_type (origin, text);
            }
          }

          void dive_into_transition ( const data::handle::transition& handle
                                    , QWidget* widget
                                    )
          {
            fhg::util::qt::first_parent_being_a<editor_window> (widget)->
              create_widget ( data::handle::function
                              ( handle.get().resolved_function()
                              , handle.document()
                              )
                            );
          }

          void nyi (const QString& what)
          {
            qDebug() << "NYI:" << what;
          }
        }

        void scene_type::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
        {
          QMenu* menu (new QMenu (event->widget()));

          if ( base_item* item_below_cursor
             = qgraphicsitem_cast<base_item*> ( itemAt ( event->scenePos()
                                                       , QTransform()
                                                       )
                                              )
             )
          {
            switch (item_below_cursor->type())
            {
            case base_item::top_level_port_graph_type:
            {
              const data::handle::port handle
                ( fhg::util::qt::throwing_qobject_cast<port_item*>
                  (item_below_cursor)->handle()
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction(tr ("port_set_name"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind ( set_name_for_handle<data::handle::port>
                              , handle
                              , tr ("port_set_name_dialog_title_for_%1").arg
                                (QString::fromStdString (handle.get().name()))
                              , tr ("port_set_name_prompt")
                              , QString::fromStdString (handle.get().name())
                              , event->widget()
                              , this
                              )
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction(tr ("port_set_type"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind ( set_we_type_for_handle<data::handle::port>
                              , handle
                              , tr ("port_set_type_dialog_title_for_%1").arg
                                (QString::fromStdString (handle.get().name()))
                              , tr ("port_set_type_prompt")
                              , QString::fromStdString (handle.get().type())
                              , event->widget()
                              , this
                              )
                );

              menu->addSeparator();

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("port_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (&data::handle::port::remove, handle, this)
                );
            }
            break;

            case base_item::port_graph_type:
            {
              const data::handle::port handle
                ( fhg::util::qt::throwing_qobject_cast<port_item*>
                  (item_below_cursor)->handle()
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction(tr ("port_set_type"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind ( set_we_type_for_handle<data::handle::port>
                              , handle
                              , tr ("port_set_type_dialog_title_for_%1").arg
                                (QString::fromStdString (handle.get().name()))
                              , tr ("port_set_type_prompt")
                              , QString::fromStdString (handle.get().type())
                              , event->widget()
                              , this
                              )
                );
            }
            break;

            case base_item::transition_graph_type:
            {
              const data::handle::transition handle
                ( fhg::util::qt::throwing_qgraphicsitem_cast<transition_item*>
                  (item_below_cursor)->handle()
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("transition_add_port"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (nyi, "transition: add port")
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction(tr ("transition_set_name"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind ( set_name_for_handle<data::handle::transition>
                              , handle
                              , tr ("transition_set_name_dialog_title_for_%1").arg
                                (QString::fromStdString (handle.get().name()))
                              , tr ("transition_set_name_prompt")
                              , QString::fromStdString (handle.get().name())
                              , event->widget()
                              , this
                              )
                );

              menu->addSeparator();

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("transition_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (&data::handle::transition::remove, handle, this)
                );

              menu->addSeparator();

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("dive_into_transition"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (dive_into_transition, handle, event->widget())
                );
            }
            break;

            case base_item::place_graph_type:
            {
              const data::handle::place handle
                ( fhg::util::qt::throwing_qgraphicsitem_cast<place_item*>
                  (item_below_cursor)->handle()
                );

              if (handle.is_implicit())
              {
                fhg::util::qt::boost_connect<void()>
                  ( menu->addAction (tr ("place_make_explicit"))
                  , SIGNAL (triggered())
                  , item_below_cursor
                  , boost::bind
                    (&data::handle::place::make_explicit, handle, this)
                  );
              }
              else
              {
                fhg::util::qt::boost_connect<void()>
                  ( menu->addAction(tr ("place_set_name"))
                  , SIGNAL (triggered())
                  , item_below_cursor
                  , boost::bind ( set_name_for_handle<data::handle::place>
                                , handle
                                , tr ("place_set_name_dialog_title_for_%1").arg
                                  (QString::fromStdString (handle.get().name()))
                                , tr ("place_set_name_prompt")
                                , QString::fromStdString (handle.get().name())
                                , event->widget()
                                , this
                                )
                  );

                fhg::util::qt::boost_connect<void()>
                  ( menu->addAction(tr ("place_set_type"))
                  , SIGNAL (triggered())
                  , item_below_cursor
                  , boost::bind ( set_we_type_for_handle<data::handle::place>
                                , handle
                                , tr ("place_set_type_dialog_title_for_%1").arg
                                  (QString::fromStdString (handle.get().name()))
                                , tr ("place_set_type_prompt")
                                , QString::fromStdString (handle.get().type())
                                , event->widget()
                                , this
                                )
                  );
              }

              menu->addSeparator();

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("place_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (&data::handle::place::remove, handle, this)
                );
            }
            break;

            case base_item::connection_graph_type:
            {
              const data::handle::connect handle
                ( fhg::util::qt::throwing_qgraphicsitem_cast<connection_item*>
                  (item_below_cursor)->handle()
                );

              if (handle.is_in())
              {
                QAction* action_read (menu->addAction(tr ("is_read_connect")));
                action_read->setCheckable (true);
                action_read->setChecked (handle.is_read());

                fhg::util::qt::boost_connect<void (bool)>
                  ( action_read
                  , SIGNAL (toggled (bool))
                , item_below_cursor
                  , boost::bind
                    (&data::handle::connect::is_read, handle, this, _1)
                  );

                menu->addSeparator();
              }

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("connection_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (&data::handle::connect::remove, handle, this)
                );
            }
            break;

            case base_item::port_place_association_graph_type:
            {
              const data::handle::port handle
                ( fhg::util::qt::throwing_qgraphicsitem_cast
                  <port_place_association*> (item_below_cursor)->handle()
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("port_place_assoc_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind ( &data::handle::port::remove_place_association
                              , handle
                              , this
                              )
                );
            }

            break;

            case base_item::place_map_graph_type:
            {
              const data::handle::place_map handle
                ( fhg::util::qt::throwing_qgraphicsitem_cast<place_map*>
                  (item_below_cursor)->handle()
                );

              fhg::util::qt::boost_connect<void()>
                ( menu->addAction (tr ("place_map_delete"))
                , SIGNAL (triggered())
                , item_below_cursor
                , boost::bind (&data::handle::place_map::remove, handle, this)
                );
            }

            break;

            default:
              //! \note Unknown item type hit. Should not happen, I guess.
              event->ignore();
              return;
            }
          }
          else
          {
            {
              QMenu* menu_new (menu->addMenu ("menu_new_element"));

              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_transition"))
                , SIGNAL (triggered())
                , this
                , boost::bind ( &data::handle::net::add_transition
                              , net()
                              , this
                              , event->scenePos()
                              )
                );

              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_place"))
                , SIGNAL (triggered())
                , this
                , boost::bind ( &data::handle::net::add_place
                              , net()
                              , this
                              , event->scenePos()
                              )
                );

              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_top_level_port_in"))
                , SIGNAL (triggered())
                , this
                , boost::bind ( &data::handle::function::add_port
                              , function()
                              , this
                              , we::type::PORT_IN
                              , event->scenePos()
                              )
                );

              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_top_level_port_out"))
                , SIGNAL (triggered())
                , this
                , boost::bind ( &data::handle::function::add_port
                              , function()
                              , this
                              , we::type::PORT_OUT
                              , event->scenePos()
                              )
                );

              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_top_level_port_tunnel"))
                , SIGNAL (triggered())
                , this
                , boost::bind ( &data::handle::function::add_port
                              , function()
                              , this
                              , we::type::PORT_TUNNEL
                              , event->scenePos()
                              )
                );

              menu_new->addSeparator();

              //! \todo Is this really needed?
              fhg::util::qt::boost_connect<void()>
                ( menu_new->addAction (tr ("new_struct"))
                , SIGNAL (triggered())
                , this
                , boost::bind (nyi, "net: new struct")
                );
            }

            menu->addSeparator();

            menu->addAction (_auto_layout_action);

            menu->addSeparator();

            fhg::util::qt::boost_connect<void()>
              ( menu->addAction (tr ("set_function_name"))
              , SIGNAL (triggered())
              , this
              , boost::bind ( set_function_name
                            , function()
                            , tr ("set_function_name_title")
                            , tr ("set_function_name_prompt")
                            , this
                            )
              );

            menu->popup (event->screenPos());
          }

          menu->connect (menu, SIGNAL (aboutToHide()), SLOT (deleteLater()));
          menu->popup (event->screenPos());

          event->accept();
        }

        void scene_type::create_pending_connection (connectable_item* item)
        {
          if (_pending_connection)
          {
            throw std::runtime_error
              ( "pending connection created while a different connection is "
              "still pending. Oo");
          }

          _pending_connection = new pending_connection (item, _mouse_position);
          _pending_connection->set_just_pos_but_not_in_property (0.0, 0.0);
          addItem (_pending_connection);

          update (_pending_connection->boundingRect());
        }

        void scene_type::create_connection (const data::handle::connect& handle)
        {
          const ::xml::parse::type::connect_type& connection (handle.get());

          connectable_item* port
            ( item_with_handle<port_item>
              ( data::handle::port ( *connection.resolved_port()
                                   , handle.document()
                                   )
              )
            );

          connectable_item* place
            ( item_with_handle<place_item>
              ( data::handle::place ( *connection.resolved_place()
                                    , handle.document()
                                    )
              )
            );

          const bool is_in (petri_net::edge::is_PT (connection.direction()));
          connectable_item* from (is_in ? place : port);
          connectable_item* to (is_in ? port : place);

          if (!to->is_connectable_with (from))
          {
            throw std::runtime_error
              ("tried hard-connecting non-connectable items.");
          }
          addItem (new connection_item (from, to, handle));
        }

        void scene_type::create_port_place_association
          (const data::handle::port& port)
        {
          if (!port.get().place)
          {
            return;
          }

          addItem
            ( new ui::graph::port_place_association
              ( item_with_handle<top_level_port_item> (port)
              , item_with_handle<place_item> ( data::handle::place
                                               ( *port.get().resolved_place()
                                               , port.document()
                                               )
                                             )
              , port
              )
            );
        }

        void scene_type::create_place_map (const data::handle::place_map& map)
        {
          const data::handle::port port
            (*map.get().resolved_tunnel_port(), map.document());
          const data::handle::place place
            (*map.get().resolved_real_place(), map.document());

          addItem ( new ui::graph::place_map
                    ( item_with_handle<port_item> (port)
                    , item_with_handle<place_item> (place)
                    , map
                    )
                  );
        }

        void scene_type::remove_pending_connection()
        {
          delete _pending_connection;
          _pending_connection = NULL;
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

          foreach ( const connectable_item* item
                  , items_of_type<connectable_item> (event->scenePos())
                  )
          {
            const connectable_item* pending (_pending_connection->fixed_end());

            if (item->is_connectable_with (pending))
            {
              const port_item* as_port
                (qobject_cast<const port_item*> (item));
              const place_item* as_place
                (qobject_cast<const place_item*> (item));

              const port_item* pending_as_port
                (qobject_cast<const port_item*> (pending));
              const place_item* pending_as_place
                (qobject_cast<const place_item*> (pending));

              if (as_port && pending_as_port)
              {
                net().add_connection_with_implicit_place
                  (this, pending_as_port->handle(), as_port->handle());
              }
              else
              {
                const port_item* port (as_port ? as_port : pending_as_port);
                const place_item* place (as_place ? as_place : pending_as_place);

                net().add_connection_or_association
                  (this, place->handle(), port->handle());
              }

              event->accept();
            }
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

        //!@{
        //! \note Dragging in transitions from the library
        //! \todo Paint a ghost transition while dragging in.
        void scene_type::dragEnterEvent (QGraphicsSceneDragDropEvent* event)
        {
          _mouse_position = event->scenePos();

          QGraphicsScene::dragEnterEvent (event);

          event->setAccepted
            (event->mimeData()->hasFormat (TransitionLibraryModel::mimeType));
        }

        void scene_type::dragMoveEvent (QGraphicsSceneDragDropEvent* event)
        {
          _mouse_position = event->scenePos();

          QGraphicsScene::dragMoveEvent (event);

          event->setAccepted
            (event->mimeData()->hasFormat (TransitionLibraryModel::mimeType));

          _mouse_position = event->scenePos();
        }

        void scene_type::dropEvent (QGraphicsSceneDragDropEvent* event)
        {
          _mouse_position = event->scenePos();

          QGraphicsScene::dropEvent (event);

          const QMimeData* mimeData (event->mimeData());

          if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
          {
            QByteArray byteArray
              (mimeData->data (TransitionLibraryModel::mimeType));
            QDataStream stream (&byteArray, QIODevice::ReadOnly);

            QSet<QString> paths;

            stream >> paths;

            foreach (const QString& path, paths)
            {
              net().add_transition
                ( this
                , data::manager::instance().load (path).get().clone
                  (boost::none, net().id().id_mapper())
                , event->scenePos()
                );

              event->acceptProposedAction();
            }
          }
        }
        //!@}

        void scene_type::auto_layout()
        {
          if (items().isEmpty())
          {
            return;
          }

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

          typedef boost::unordered_map < association*
                                       , graphviz::edge_type
                                       > edges_map_type;
          edges_map_type edges;

          foreach (association* c, items_of_type<association>())
          {
            QGraphicsItem* start (c->start());
            QGraphicsItem* end (c->end());

            start = start->parentItem() ? start->parentItem() : c->start();
            end = end->parentItem() ? end->parentItem() : c->end();

            nodes_map_type::iterator start_node
              (nodes.find (qgraphicsitem_cast<base_item*> (start)));
            nodes_map_type::iterator end_node
              (nodes.find (qgraphicsitem_cast<base_item*> (end)));

            if (start_node != nodes.end() && end_node != nodes.end())
            {
              edges.insert
                ( edges_map_type::value_type
                  (c, graph.add_edge (start_node->second, end_node->second))
                );
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

        template<typename item_type, typename handle_type>
          item_type* scene_type::item_with_handle (const handle_type& handle)
        {
          foreach (item_type* item, items_of_type<item_type>())
          {
            if (item->handle() == handle)
            {
              return item;
            }
          }
          return NULL;
        }

        template<typename handle_type>
          bool scene_type::is_in_my_net (const handle_type& handle)
        {
          return handle.get().parent()->id() == net().id();
        }

        template<>
          bool scene_type::is_in_my_net (const data::handle::connect& handle)
        {
          return handle.get().parent()->parent()->id() == net().id();
        }

        template<>
          bool scene_type::is_in_my_net (const data::handle::port& handle)
        {
          return handle.get().parent()->id() == function().id();
        }
        template<>
          bool scene_type::is_in_my_net (const data::handle::place_map& handle)
        {
          return handle.get().resolved_real_place()->get().parent()->id()
            == net().id();
        }

        const data::handle::net& scene_type::net() const
        {
          return _net;
        }

        const data::handle::function& scene_type::function() const
        {
          return _function;
        }

        template<typename item_type>
          QList<item_type*> scene_type::items_of_type() const
        {
          QList<item_type*> result;

          foreach (QGraphicsItem* child, items())
          {
            base_item* bi =
              fhg::util::qt::throwing_qgraphicsitem_cast<base_item*> (child);
            if (item_type* item = qobject_cast<item_type*> (bi))
            {
              result << item;
            }
          }

          return result;
        }

        template<typename item_type>
          QList<item_type*> scene_type::items_of_type (const QPointF& pos) const
        {
          QList<item_type*> result;

          foreach (QGraphicsItem* child, items (pos))
          {
            base_item* bi =
              fhg::util::qt::throwing_qgraphicsitem_cast<base_item*> (child);
            if (item_type* item = qobject_cast<item_type*> (bi))
            {
              result << item;
            }
          }

          return result;
        }

        template<typename item_type, typename handle_type>
          void scene_type::remove_item_for_handle (const handle_type& handle)
        {
          if (is_in_my_net (handle))
          {
            item_type* item (item_with_handle<item_type> (handle));
            removeItem (item);
            delete item;
          }
        }

        // # connection ##############################################
        //! \todo Don't pass from and to. Pass net.
        void scene_type::connection_added
          ( const QObject* origin
          , const data::handle::connect& connection
          , const data::handle::place& place
          , const data::handle::port& port
          )
        {
          if (is_in_my_net (place))
          {
            create_connection (connection);
          }
        }

        void scene_type::connection_removed
          (const QObject* origin, const data::handle::connect& connection)
        {
          remove_item_for_handle<connection_item> (connection);
        }

        // # place_map ##############################################
        //! \todo Don't pass from and to. Pass net.
        void scene_type::place_map_added
          (const QObject* origin, const data::handle::place_map& place_map)
        {
          if (is_in_my_net (place_map))
          {
            create_place_map (place_map);
          }
        }

        void scene_type::place_map_removed
          (const QObject* origin, const data::handle::place_map& handle)
        {
          remove_item_for_handle<place_map> (handle);
        }

        // # transition ##############################################
        void scene_type::transition_added
          (const QObject* origin, const data::handle::transition& transition)
        {
          if (is_in_my_net (transition))
          {
            weaver::display::transition (transition, this);
          }
        }

        void scene_type::transition_deleted
          (const QObject* origin, const data::handle::transition& transition)
        {
          remove_item_for_handle<transition_item> (transition);
        }

        // # place ###################################################
        void scene_type::place_added
          (const QObject* origin, const data::handle::place& place)
        {
          if (is_in_my_net (place))
          {
            weaver::display::place (place, this);
          }
        }

        void scene_type::place_deleted
          (const QObject* origin, const data::handle::place& place)
        {
          remove_item_for_handle<place_item> (place);
        }


        // # port ####################################################
        void scene_type::port_added
          (const QObject* origin, const data::handle::port& port)
        {
          if (is_in_my_net (port))
          {
            weaver::display::top_level_port (port, this);
          }
        }

        void scene_type::port_deleted
          (const QObject* origin, const data::handle::port& port)
        {
          if (port.is_tunnel())
          {
            return;
          }

          remove_item_for_handle<port_item> (port);
        }

        void scene_type::place_association_set
          ( const QObject* origin
          , const data::handle::port& port
          , const boost::optional<std::string>& place
          )
        {
          if (port.is_tunnel())
          {
            return;
          }

          if (place)
          {
            top_level_port_item* port_item (NULL);

            if ( port_place_association* assoc_item
               = item_with_handle<port_place_association> (port)
               )
            {
              port_item =
                fhg::util::qt::throwing_qobject_cast<top_level_port_item*>
                  (assoc_item->start());

              removeItem (assoc_item);
              delete assoc_item;
            }

            if (!port_item)
            {
              port_item = item_with_handle<top_level_port_item> (port);
              if (!port_item)
              {
                throw std::runtime_error ("place_association for unknown port");
              }
            }

            foreach (place_item* item, items_of_type<place_item>())
            {
              if (item->handle().get().name() == *place)
              {
                addItem (new port_place_association (port_item, item, port));
                return;
              }
            }
            throw std::runtime_error ("place_association to unknown place");
          }
          else
          {
            remove_item_for_handle<port_place_association> (port);
          }
        }
      }
    }
  }
}
