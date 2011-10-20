// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/item.hpp>

#include <QGraphicsSceneHoverEvent>

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
        {
          _mode.push (style::mode::NORMAL);
          setAcceptHoverEvents (true);
        }

        class scene* item::scene() const
        {
          return
            fhg::util::throwing_qobject_cast<class scene*>(QGraphicsItem::scene());
        }


        void item::setPos (qreal x, qreal y)
        {
          detail::set_position_x (_property, x);
          detail::set_position_y (_property, y);

          QGraphicsItem::setPos (x, y);
        }
        void item::setPos (const QPointF& pos)
        {
          detail::set_position_x (_property, pos.x());
          detail::set_position_y (_property, pos.y());

          QGraphicsItem::setPos (pos);
        }

        void item::set_just_pos_but_not_in_property (qreal x, qreal y)
        {
          QGraphicsItem::setPos (x, y);
        }
        void item::set_just_pos_but_not_in_property (const QPointF& pos)
        {
          QGraphicsItem::setPos (pos);
        }

        style::type& item::access_style () { return _style; }

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

        void item::mode_push (const style::mode::type& mode)
        {
          _mode.push (mode);
          update();
        }
        void item::mode_pop ()
        {
          _mode.pop();
          update();
        }
        const style::mode::type& item::mode() const
        {
          if (_mode.empty())
            {
              throw std::runtime_error ("STRANGE: No style mode!?");
            }

          return _mode.top();
        }

        void item::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_pop();
          update (boundingRect());
        }
        void item::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_push (style::mode::HIGHLIGHT);
          update (boundingRect());
        }
      }
    }
  }
}
