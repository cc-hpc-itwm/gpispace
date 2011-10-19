// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_HPP 1

#include <pnete/ui/graph/style/type.fwd.hpp>
#include <pnete/ui/graph/style/fallback.hpp>
#include <pnete/ui/graph/style/store.hpp>

#include <boost/unordered_map.hpp>

#include <QGraphicsItem>

class QPainter;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class item;

        namespace style
        {
          namespace detail
          {
            template<typename T, typename Item>
            const T& fallback (const Item* item, const key_type& key)
            {
              if ( const Item* parent
                 = qgraphicsitem_cast<const Item*> (item->parentItem())
                 )
                {
                  return parent->style().template get<T> (parent, key);
                }

              return fallback::get<T> (key);
            }
          }

          namespace store
          {
            template<typename T>
            class of
            {
            public:
              typedef cached_predicates<const graph::item*, T> type;
            };

            typedef boost::variant < of<qreal>::type
                                   , of<QColor>::type
                                   > type;

            namespace visitor
            {
              template<typename T>
              class mk : public boost::static_visitor<type&>
              {
              private:
                type& _x;

              public:
                mk (type& x) : _x (x) {}

                type& operator () (T&) const { return _x; }

                template<typename U>
                type& operator () (const U&) const { return _x = T(); }
              };
            }

            template<typename T> type& mk_or_keep (type& x)
            {
              return boost::apply_visitor (visitor::mk<T> (x), x);
            }
          }

          class type
          {
          private:
            typedef boost::unordered_map<key_type, store::type> map_type;

            map_type _store_for;

          public:
            template<typename T>
            void
            push ( const key_type& key
                 , const typename store::of<T>::type::predicate_type& pred
                 )
            {
              typedef typename store::of<T>::type store_type;

              boost::get<store_type&>
                (store::mk_or_keep<store_type> (_store_for[key])).push (pred);
            }

            template<typename T>
            const T&
            get (const graph::item* item, const key_type& key) const
            {
              typedef typename store::of<T>::type store_type;

              const map_type::const_iterator pos (_store_for.find (key));

              if (pos == _store_for.end())
                {
                  return detail::fallback<T, graph::item> (item, key);
                }

              typename store::of<T>::type::optional_value_type value
                (boost::get<const store_type&> (pos->second).get (item));

              if (value)
                {
                  return *value;
                }

              return detail::fallback<T, graph::item> (item, key);
            }
          };

          void draw_shape (const type&, const graph::item*, QPainter* painter);
        }
      }
    }
  }
}

#endif
