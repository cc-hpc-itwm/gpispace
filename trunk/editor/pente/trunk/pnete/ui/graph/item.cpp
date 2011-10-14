// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/item.hpp>

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
        {}

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
      }
    }
  }
}
