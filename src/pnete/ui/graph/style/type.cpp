// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/type.hpp>

#include <pnete/ui/graph/base_item.hpp>

#include <QPainter>
#include <QPen>
#include <QBrush>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace style
        {
          void draw_shape ( const base_item* item
                          , QPainter* painter
                          )
          {
            painter->setPen
              (QPen ( QBrush (item->style<QColor> ("border_color"))
                    , item->style<qreal> ("border_thickness")
                    , item->style<Qt::PenStyle> ("border_style")
                    )
              );
            painter->setBackgroundMode (Qt::OpaqueMode);
            painter->setBrush (QBrush (item->style<QColor> ("background_color")));
            painter->drawPath (item->shape());
          }

          namespace store
          {
            namespace
            {
              class visitor_clear_cache : public boost::static_visitor<void>
              {
              public:
                template<typename T>
                void operator () (const T& x) const { x.clear_cache(); }
              };
            }

            void clear_cache (const type& x)
            {
              boost::apply_visitor (visitor_clear_cache(), x);
            }
          }

          void type::clear_cache ()
          {
            BOOST_FOREACH( const by_mode_by_key_type::value_type& by_mode
                         , _by_mode_by_key
                         )
              {
                BOOST_FOREACH( const by_mode_type::value_type& store
                             , by_mode.second
                             )
                  {
                    store::clear_cache (store.second);
                  }
              }
          }
        }
      }
    }
  }
}
