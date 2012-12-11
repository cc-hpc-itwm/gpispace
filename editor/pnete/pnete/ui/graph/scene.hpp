// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_SCENE_HPP
#define _FHG_PNETE_UI_GRAPH_SCENE_HPP 1

#include <pnete/ui/graph/scene.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/connect.fwd.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>
#include <pnete/data/internal.fwd.hpp>
#include <pnete/ui/graph/base_item.fwd.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>
#include <pnete/ui/graph/connection.fwd.hpp>
#include <pnete/ui/graph/pending_connection.fwd.hpp>
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
          Q_OBJECT;

        public:
          explicit scene_type ( const data::handle::net& net
                              , data::internal_type* internal
                              , QObject* parent = NULL
                              );

          void create_pending_connection (connectable_item* item);
          void create_connection ( connectable_item* from
                                 , connectable_item* to
                                 , const data::handle::connect& handle
                                 );

          data::internal_type* internal() const;
          data::change_manager_t& change_manager() const;

        public slots:
          void slot_add_struct ();
          void auto_layout();

          // ## trigger modification #################################
          // # transition ############################################
          void slot_add_transition() const;

          // # place #################################################
          void slot_add_place() const;

          // ## react on modification ################################
          // # connection ############################################
          void connection_added ( const QObject*
                                , const data::handle::connect&
                                , const data::handle::place&
                                , const data::handle::port&
                                );
          void connection_removed ( const QObject*
                                  , const data::handle::connect&
                                  );

          // # transition ############################################
          void transition_added (const QObject*, const data::handle::transition&);
          void transition_deleted (const QObject*, const data::handle::transition&);

          // # place #################################################
          void place_added (const QObject*, const data::handle::place&);
          void place_deleted (const QObject*, const data::handle::place&);

          // # top-level-port ########################################
          void place_association_set
            ( const QObject* origin
            , const data::handle::port& port
            , const boost::optional<std::string>& place
            );

        protected:
          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent (QKeyEvent* event);

        private:
          template<typename item_type, typename handle_type>
            item_type* item_with_handle (const handle_type&);

          template<typename handle_type>
            bool is_in_my_net (const handle_type&);

          template<typename item_type> QList<item_type*> items_of_type() const;
          template<typename item_type> QList<item_type*> items_of_type
            (const QPointF&) const;

          template<typename item_type, typename handle_type>
            void remove_item_for_handle (const handle_type& handle);

          const data::handle::net& net() const;

          void remove_transition_item (transition_item*);

          void remove_pending_connection();

          void init_menu_context();

          pending_connection* _pending_connection;
          QPointF _mouse_position;

          QMenu _menu_context;

          data::handle::net _net;
          data::internal_type* _internal;

          friend class fhg::pnete::ui::GraphView; // for net() only.
        };
      }
    }
  }
}

#endif
