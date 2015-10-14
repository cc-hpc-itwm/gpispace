// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/graph/scene.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/connect.fwd.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/place_map.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>
#include <pnete/data/manager.fwd.hpp>
#include <pnete/ui/graph/base_item.fwd.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>
#include <pnete/ui/graph/connection.fwd.hpp>
#include <pnete/ui/graph/pending_connection.fwd.hpp>
#include <pnete/ui/graph/place.fwd.hpp>
#include <pnete/ui/graph/transition.fwd.hpp>
#include <pnete/ui/graph_view.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>

#include <QGraphicsScene>
#include <QMenu>
#include <QObject>
#include <QPointF>

#include <string>

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QMenu;
class QRectF;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class scene_type : public QGraphicsScene
        {
          Q_OBJECT

        public:
          explicit scene_type ( data::manager&
                              , const data::handle::net& net
                              , const data::handle::function& function
                              , QObject* parent = nullptr
                              );

          void create_pending_connection (connectable_item* item);
          void create_connection (const data::handle::connect&);
          void create_port_place_association (const data::handle::port&);
          void create_place_map (const data::handle::place_map&);

          QList<QAction*> actions() const;

        public slots:
          void auto_layout();

          void connection_added ( const data::handle::connect&
                                , const data::handle::place&
                                , const data::handle::port&
                                );
          void place_map_added (const data::handle::place_map&);
          void transition_added (const data::handle::transition&);
          void place_added (const data::handle::place&);
          void port_added (const data::handle::port&);
          void place_association_set
            ( const data::handle::port&
            , const boost::optional<std::string>& place
            );

        protected:
          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event) override;
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent) override;
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event) override;
          virtual void keyPressEvent (QKeyEvent* event) override;
          virtual void dragEnterEvent (QGraphicsSceneDragDropEvent* event) override;
          virtual void dragMoveEvent (QGraphicsSceneDragDropEvent* event) override;
          virtual void dropEvent (QGraphicsSceneDragDropEvent* event) override;

        private:
          template<typename item_type, typename handle_type>
            item_type* item_with_handle (const handle_type&);

          template<typename item_type> QList<item_type*> items_of_type() const;
          template<typename item_type> QList<item_type*> items_of_type
            (const QPointF&) const;

          const data::handle::net& net() const;
          const data::handle::function& function() const;

          void remove_pending_connection();

          QAction* connect_action (QAction*, const char* slot);
          template<typename FUN_TYPE>
          QAction* connect_action (QAction*, FUN_TYPE);

          pending_connection* _pending_connection;
          QPointF _mouse_position;

          data::manager& _data_manager;
          data::handle::net _net;
          data::handle::function _function;

          QAction* _add_transition_action;
          QAction* _add_place_action;
          QAction* _add_top_level_port_in_action;
          QAction* _add_top_level_port_out_action;
          QAction* _add_top_level_port_tunnel_action;
          QAction* _auto_layout_action;
          QList<QAction*> _actions;
        };
      }
    }
  }
}
