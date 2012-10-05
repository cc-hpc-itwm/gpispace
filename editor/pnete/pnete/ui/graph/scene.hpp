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
    namespace data { class change_manager_t; }

    namespace ui
    {
      namespace graph
      {
        class item;
        class connectable_item;
        class transition_item;
        namespace connection { class item; }

        namespace scene
        {
          class type : public QGraphicsScene
          {
            Q_OBJECT;

          public:
            explicit type ( ::xml::parse::type::net_type& net
                          , data::change_manager_t& change_manager
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
            void slot_delete_transition (graph::item*);
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

            void slot_delete_place (graph::item*);
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

            connection::item* create_connection (bool only_reading = false);
            void remove_pending_connection();

            void init_menu_context();

            connection::item* _pending_connection;
            QPointF _mouse_position;

            QMenu _menu_new;
            QMenu _menu_context;

            ::xml::parse::type::net_type& _net;
            data::change_manager_t& _change_manager;
          };
        }
      }
    }
  }
}

#endif
