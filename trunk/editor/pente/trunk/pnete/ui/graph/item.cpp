// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/item.hpp>

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/util.hpp>
#include <util/property.hpp>

#include <we/type/property.hpp>

#include <boost/lexical_cast.hpp>

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

        item::item ( item* parent
                   , ::we::type::property::type* property
                   )
          : QGraphicsItem (parent)
          , _property (property)
          , _style ()
          , _mode ()
          , _move_start ()
        {
          _mode.push (mode::NORMAL);
          setAcceptHoverEvents (true);
          setAcceptedMouseButtons (Qt::LeftButton);
        }

        scene::type* item::scene() const
        {
          QGraphicsScene* sc (QGraphicsItem::scene());

          return sc
            ? fhg::util::throwing_qobject_cast<scene::type*> (sc)
            : 0
            ;
        }

        void item::setPos (const QPointF& new_pos)
        {
          QPointF snapped (style::raster::snap (new_pos));

          detail::set_position_x (_property, snapped.x());
          detail::set_position_y (_property, snapped.y());

          set_just_pos_but_not_in_property (snapped);
        }
        void item::set_just_pos_but_not_in_property (qreal x, qreal y)
        {
          set_just_pos_but_not_in_property (QPointF (x, y));
        }
        void item::set_just_pos_but_not_in_property (const QPointF& new_pos)
        {
          foreach (item* child, childs())
            {
              child->setVisible (false);
            }

          QGraphicsItem::setPos (new_pos);

          foreach (item* child, childs())
            {
              child->setVisible (true);
            }
        }

        //! \todo remove me
        style::type& item::access_style ()
        {
          return _style;
        }

        void item::clear_style_cache ()
        {
          _style.clear_cache();

          foreach (QGraphicsItem* child, childItems())
            {
              if (item* child_item = qgraphicsitem_cast<item*> (child))
                {
                  child_item->clear_style_cache();
                }
            }
        }

        void item::mode_push (const mode::type& mode)
        {
          _mode.push (mode);
          update ();
        }
        void item::mode_pop ()
        {
          _mode.pop();
          update ();
        }
        const mode::type& item::mode() const
        {
          if (_mode.empty())
            {
              throw std::runtime_error ("STRANGE: No mode!?");
            }

          return _mode.top();
        }

        void item::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_push (mode::HIGHLIGHT);
        }
        void item::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_pop();
        }
        void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier)
            {
              mode_push (mode::MOVE);
              _move_start = event->pos();
            }
        }
        void item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              mode_pop();
            }
        }
        void item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              setPos (pos() + event->pos() - _move_start);
            }
        }

        QLinkedList<item*> item::childs () const
        {
          QLinkedList<item*> childs;

          foreach (QGraphicsItem* childItem, childItems())
            {
              if (item* child = qgraphicsitem_cast<item *> (childItem))
                {
                  childs << child << child->childs();
                }
            }

          return childs;
        }

        QRectF item::boundingRect () const
        {
          return shape().controlPointRect();
        }
      }
    }
  }
}
