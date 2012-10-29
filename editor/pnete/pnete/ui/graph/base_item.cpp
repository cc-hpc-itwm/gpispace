// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/base_item.hpp>

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/util.hpp>
#include <util/property.hpp>

#include <we/type/property.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace detail
        {
          static void set_position_x ( ::we::type::property::type* prop
                                     , const qreal& x
                                     )
          {
            static util::property::setter s ("position.x"); s.set (prop, x);
          }
          static void set_position_y ( ::we::type::property::type* prop
                                     , const qreal& x
                                     )
          {
            static util::property::setter s ("position.y"); s.set (prop, x);
          }
        }

        base_item::base_item (base_item* parent)
          : QGraphicsObject (parent)
          , _style ()
          , _mode ()
          , _move_start ()
        {
          _mode.push (mode::NORMAL);
          setAcceptHoverEvents (true);
          setAcceptedMouseButtons (Qt::LeftButton);
        }

        scene_type* base_item::scene() const
        {
          QGraphicsScene* sc (QGraphicsItem::scene());

          return sc ? fhg::util::throwing_qobject_cast<scene_type*> (sc)
                    : NULL
                    ;
        }

        void base_item::setPos (const QPointF& new_pos)
        {
          QPointF snapped (style::raster::snap (new_pos));

          // detail::set_position_x (_property, snapped.x());
          // detail::set_position_y (_property, snapped.y());

          set_just_pos_but_not_in_property (snapped);
        }
        void base_item::set_just_pos_but_not_in_property (qreal x, qreal y)
        {
          set_just_pos_but_not_in_property (QPointF (x, y));
        }
        void base_item::set_just_pos_but_not_in_property (const QPointF& new_pos)
        {
          //! \todo update more clever
          foreach (base_item* child, childs())
            {
              child->setVisible (false);
            }

          QGraphicsItem::setPos (new_pos);

          foreach (base_item* child, childs())
            {
              child->setVisible (true);
            }
        }

        //! \todo remove me
        style::type& base_item::access_style ()
        {
          return _style;
        }

        void base_item::clear_style_cache ()
        {
          _style.clear_cache();

          foreach (QGraphicsItem* child, childItems())
            {
              if (base_item* child_item = qgraphicsitem_cast<base_item*> (child))
                {
                  child_item->clear_style_cache();
                }
            }
        }

        void base_item::mode_push (const mode::type& mode)
        {
          _mode.push (mode);
          update ();
        }
        void base_item::mode_pop ()
        {
          _mode.pop();
          update ();
        }
        const mode::type& base_item::mode() const
        {
          if (_mode.empty())
            {
              throw std::runtime_error ("STRANGE: No mode!?");
            }

          return _mode.top();
        }

        void base_item::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_push (mode::HIGHLIGHT);
        }
        void base_item::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_pop();
        }
        void base_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier)
            {
              mode_push (mode::MOVE);
              _move_start = event->pos();
            }
        }
        void base_item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              mode_pop();
            }
        }
        void base_item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              setPos (pos() + event->pos() - _move_start);
            }
        }

        QLinkedList<base_item*> base_item::childs () const
        {
          QLinkedList<base_item*> childs;

          foreach (QGraphicsItem* childItem, childItems())
            {
              if (base_item* child = qgraphicsitem_cast<base_item *> (childItem))
                {
                  childs << child << child->childs();
                }
            }

          return childs;
        }

        QRectF base_item::boundingRect () const
        {
          return shape().controlPointRect();
        }
      }
    }
  }
}
