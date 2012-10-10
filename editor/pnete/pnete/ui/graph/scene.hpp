// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_SCENE_HPP
#define _FHG_PNETE_UI_GRAPH_SCENE_HPP 1

#include <xml/parse/types.hpp>

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
          explicit scene_type ( ::xml::parse::type::net_type& net
                              , data::internal_type* internal
                              , QObject* parent = NULL
                              );
          
          const QPointF& mouse_position() const;

          void create_connection (connectable_item* item);
          void create_connection ( connectable_item* from
                                 , connectable_item* to
                                 , bool only_reading
                                 );

          ::xml::parse::type::net_type& net();
          data::change_manager_t& change_manager();

        public slots:
          void slot_delete_transition (base_item*);
          void
          slot_delete_transition ( const QObject*
                                 , const ::xml::parse::type::transition_type&
                                 , const ::xml::parse::type::net_type&
                                 );

          void slot_add_transition ();
          void slot_add_transition ( const QObject*
                                   , ::xml::parse::type::transition_type&
                                   , ::xml::parse::type::net_type&
                                   );
          void slot_add_place ();
          void slot_add_place ( const QObject*
                              , ::xml::parse::type::place_type&
                              , ::xml::parse::type::net_type&
                              );

          void slot_delete_place (base_item*);
          void
          slot_delete_place ( const QObject*
                            , const ::xml::parse::type::place_type&
                            , const ::xml::parse::type::net_type&
                            );


          void slot_add_struct ();

          void auto_layout();

        signals:

        protected:
          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void keyPressEvent (QKeyEvent* event);

        private:
          void remove_transition_item (transition_item*);
          bool is_my_net (const ::xml::parse::type::net_type&);

          connection_item* create_connection (bool only_reading = false);
          void remove_pending_connection();

          void init_menu_context();

          connection_item* _pending_connection;
          QPointF _mouse_position;

          QMenu _menu_new;
          QMenu _menu_context;

          ::xml::parse::type::net_type& _net;
          data::internal_type* _internal;
        };
      }
    }
  }
}

#endif
