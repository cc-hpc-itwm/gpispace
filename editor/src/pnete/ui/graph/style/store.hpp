// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_STORE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_STORE_HPP 1

#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <list>

#include <QColor>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class base_item;

        namespace style
        {
          namespace store
          {
            template<typename Key, typename Value>
            class cached_predicates
            {
            public:
              typedef boost::optional<Value> optional_value_type;
              typedef boost::function< optional_value_type (Key)
                                     > predicate_type;

            private:
              typedef boost::unordered_map< Key
                                          , optional_value_type
                                          > cache_type;
              typedef std::list<predicate_type> predicates_type;

              typedef typename cache_type::const_iterator cache_iterator;
              typedef typename cache_type::value_type cache_entry_type;
              typedef typename predicates_type::const_iterator
                               predicates_iterator;

            public:
              cached_predicates ()
                : _cache ()
                , _predicates ()
              {}
              explicit cached_predicates (const predicate_type& predicate)
                : _cache ()
                , _predicates ()
              {
                _predicates.push_back (predicate);
              }

              void clear_cache () const
              {
                _cache.clear();
              }

              //! \todo implement other accessors
              void push (const predicate_type& predicate)
              {
                _predicates.push_back (predicate);
                clear_cache();
              }

              optional_value_type get (Key key) const
              {
                const cache_iterator cache_pos (_cache.find (key));

                if (cache_pos != _cache.end())
                  {
                    return cache_pos->second;
                  }

                predicates_iterator predicate (_predicates.begin());
                const predicates_iterator no_predicate (_predicates.end());

                while (predicate != no_predicate)
                  {
                    optional_value_type value ((*predicate) (key));

                    if (value)
                      {
                        return cache (key, value);
                      }

                    ++predicate;
                  }

                return cache (key, boost::none);
              }

            private:
              mutable cache_type _cache;
              predicates_type _predicates;

              const optional_value_type&
              cache ( Key key
                    , const optional_value_type& value
                    ) const
              {
                _cache.insert (cache_entry_type (key, value));

                return value;
              }
            };
          }
        }
      }
    }
  }
}

#endif
