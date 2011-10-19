// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_ITEM_HPP
#define _PNETE_UI_GRAPH_ITEM_HPP 1

#include <QObject>
#include <QGraphicsItem>

#include <pnete/ui/graph/scene.hpp>

#include <pnete/ui/graph/orientation.hpp>

#include <pnete/ui/graph/style/type.hpp>
#include <pnete/ui/graph/style/mode.hpp>

class QGraphicsSceneHoverEvent;

#include <stack>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class scene;

      namespace graph
      {
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

          class scene* scene() const;

          void setPos (const QPointF&);
          void setPos (qreal, qreal);

          void set_just_pos_but_not_in_property (const QPointF&);
          void set_just_pos_but_not_in_property (qreal, qreal);

          virtual void set_just_orientation_but_not_in_property
          (const port::orientation::type&) {}

          //! \todo eliminate write acces to _style
          style::type& access_style();

          void clear_style_cache();

          template<typename T>
          const T& style (const style::key_type& key) const
          {
            return _style.get<T> (this, mode(), key);
          }

          const style::mode::type& mode() const;

        private:
          ::we::type::property::type* _property;
          style::type _style;
          std::stack<style::mode::type> _mode;

        protected:
          virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event);
        };
      }
    }
  }
}

#endif
