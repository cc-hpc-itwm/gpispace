// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_ITEM_HPP
#define _PNETE_UI_GRAPH_ITEM_HPP 1

#include <QObject>
#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>
#include <QLinkedList>

#include <pnete/ui/graph/scene.hpp>

#include <pnete/ui/graph/orientation.hpp>

#include <pnete/ui/graph/style/type.hpp>
#include <pnete/ui/graph/mode.hpp>

class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;

#include <stack>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace scene { class type; }

        class item : public QObject, public QGraphicsItem
        {
          Q_OBJECT;
          Q_INTERFACES(QGraphicsItem);

        public:
          enum ItemTypes
          {
            connection_graph_type       = QGraphicsItem::UserType + 1,
            port_graph_type             = QGraphicsItem::UserType + 2,
            transition_graph_type       = QGraphicsItem::UserType + 3,
            place_graph_type            = QGraphicsItem::UserType + 4,
            top_level_port_graph_type   = QGraphicsItem::UserType + 5,
            /* ... */
          };

          item ( item* parent = NULL
               , ::we::type::property::type* property = NULL
               );

          scene::type* scene() const;

          void set_just_pos_but_not_in_property (const QPointF&);
          void set_just_pos_but_not_in_property (qreal, qreal);

          virtual void set_just_orientation_but_not_in_property
          (const port::orientation::type&) {}

          virtual void setPos (const QPointF&);
          virtual void setPos (qreal, qreal);

          //! \todo eliminate write acces to _style
          style::type& access_style();

          void clear_style_cache();

          template<typename T>
          const T& style (const style::key_type& key) const
          {
            return _style.get<T> (this, mode(), key);
          }

          const mode::type& mode() const;
          void mode_push (const mode::type&);
          void mode_pop();

          virtual QLinkedList<item*> childs() const;

        private:
          ::we::type::property::type* _property;
          style::type _style;
          std::stack<mode::type> _mode;
          QPointF _move_start;

        protected:
          virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
        };
      }
    }
  }
}

#endif
