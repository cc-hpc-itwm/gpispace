// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_SCENE_HPP
#define _FHG_PNETE_UI_GRAPH_SCENE_HPP 1

#include <pnete/ui/graph/scene.fwd.hpp>

//! \todo Remove
#include <xml/parse/type/net.fwd.hpp>

#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>

#include <QGraphicsScene>
#include <QPointF>
#include <QObject>
#include <QMenu>

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QRectF;
class QMenu;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class change_manager_t;
      class internal_type;
    }

    namespace ui
    {
      class GraphView;

      namespace graph
      {
        class base_item;
        class connectable_item;
        class connection_item;
        class transition_item;

        class scene_type : public QGraphicsScene
        {
          Q_OBJECT;

        public:
          explicit scene_type ( const data::handle::net& net
                              , data::internal_type* internal
                              , QObject* parent = NULL
                              );

          const QPointF& mouse_position() const;

          void create_connection (connectable_item* item);
          void create_connection ( connectable_item* from
                                 , connectable_item* to
                                 , bool only_reading
                                 );

          data::change_manager_t& change_manager();

        public slots:
          void slot_add_struct ();
          void auto_layout();

          // ## trigger modification #################################
          // # transition ############################################
          void slot_add_transition ();
          void slot_delete_transition (base_item*);

          // # place #################################################
          void slot_add_place ();
          void slot_delete_place (base_item*);

          // ## react on modification ################################
          // # transition ############################################
          void transition_added (const QObject*, const data::handle::transition&);
          void transition_deleted (const QObject*, const data::handle::transition&);

          // # place #################################################
          void place_added (const QObject*, const data::handle::place&);
          void place_deleted (const QObject*, const data::handle::place&);

        protected:
          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent (QKeyEvent* event);

        private:
          void change_mgr_link
            (const char* signal, const char* slot, const char* arguments);

          template<typename item_type, typename handle_type>
            item_type* item_with_handle (const handle_type&);

          template<typename handle_type>
            bool is_in_my_net (const handle_type&);

          const data::handle::net& net() const;

          void remove_transition_item (transition_item*);

          connection_item* create_connection (bool only_reading = false);
          void remove_pending_connection();

          void init_menu_context();

          connection_item* _pending_connection;
          QPointF _mouse_position;

          QMenu _menu_new;
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
