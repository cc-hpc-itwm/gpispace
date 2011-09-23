// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_SCENE_HPP
#define _FHG_PNETE_UI_GRAPH_SCENE_HPP 1

#include <pnete/data/internal.hpp>

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
        class connection;
        class connectable_item;

        class scene : public QGraphicsScene
        {
          Q_OBJECT;

        public:
          typedef ::xml::parse::type::net_type net_type;

          explicit scene (net_type & net,  QObject* parent = NULL);

          const QPointF& mouse_position() const;

          void create_connection (connectable_item* item);
          void create_connection ( connectable_item* from
                                 , connectable_item* to
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
          connection* create_connection (bool only_reading = false);
          void remove_pending_connection();

          void init_menu_context();

          connection* _pending_connection;
          QPointF _mouse_position;

          QMenu _menu_new;
          QMenu _menu_context;

          net_type & _net;
        };
      }
    }
  }
}

#endif
