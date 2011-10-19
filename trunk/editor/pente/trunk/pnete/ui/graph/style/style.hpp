// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_HPP 1

#include <pnete/ui/graph/style/style.fwd.hpp>
#include <pnete/ui/graph/style/fallback.hpp>
#include <pnete/ui/graph/style/store.hpp>

#include <boost/unordered_map.hpp>

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
            typename store::of<T>::type::optional_value_type
            get (const graph::item* item, const key_type& key) const
            {
              typedef typename store::of<T>::type store_type;

              const map_type::const_iterator pos (_store_for.find (key));

              if (pos == _store_for.end())
                {
                  return boost::none;
                }

              return boost::get<const store_type&> (pos->second).get (item);
            }

            template<typename T>
            const T&
            get ( const graph::item* item
                , const key_type& key
                , const fallback::type& fallback
                ) const
            {
              const typename store::of<T>::type::optional_value_type value
                (get<T> (item, key));

              return value ? *value : fallback.get<T> (key);
            }
          };
        }
      }
    }
  }
}

#endif
