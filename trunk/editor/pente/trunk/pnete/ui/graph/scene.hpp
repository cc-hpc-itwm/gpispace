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
    namespace ui
    {
      namespace graph
      {
        namespace connection { class item; }
        namespace connectable { class item; }

        namespace scene
        {
          class type : public QGraphicsScene
          {
            Q_OBJECT;

          public:
            typedef ::xml::parse::type::net_type net_type;

            explicit type (net_type & net,  QObject* parent = NULL);

            const QPointF& mouse_position() const;

            void create_connection (connectable::item* item);
            void create_connection ( connectable::item* from
                                   , connectable::item* to
                                   , bool only_reading
                                   );

          public slots:
            void slot_add_transition ();
            void slot_add_place ();
            void slot_add_struct ();

            void auto_layout();

          protected:
            virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
            virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
            virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
            virtual void keyPressEvent (QKeyEvent* event);

          private:
            connection::item* create_connection (bool only_reading = false);
            void remove_pending_connection();

            void init_menu_context();

            connection::item* _pending_connection;
            QPointF _mouse_position;

            QMenu _menu_new;
            QMenu _menu_context;

            net_type & _net;
          };
        }
      }
    }
  }
}

#endif
