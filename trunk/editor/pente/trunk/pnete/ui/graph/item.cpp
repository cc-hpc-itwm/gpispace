// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/item.hpp>

#include <pnete/util.hpp>

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
          class property_setter
          {
          private:
            ::we::type::property::path_type _path;

          public:
            explicit property_setter (const ::we::type::property::key_type& p)
              : _path (::we::type::property::util::split (p))
            {}

            template<typename T>
            void set (::we::type::property::type* prop, T x)
            {
              if (prop)
                {
                  prop->set
                    ( _path.begin()
                    , _path.end()
                    , boost::lexical_cast< ::we::type::property::value_type> (x)
                    );
                }
            }
          };

          static void set_position_x ( ::we::type::property::type* prop
                                     , const qreal& x
                                     )
          {
            static property_setter p ("fhg.pnete.position.x"); p.set (prop, x);
          }
          static void set_position_y ( ::we::type::property::type* prop
                                     , const qreal& x
                                     )
          {
            static property_setter p ("fhg.pnete.position.y"); p.set (prop, x);
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
            util::throwing_qobject_cast<class scene*>(QGraphicsItem::scene());
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
