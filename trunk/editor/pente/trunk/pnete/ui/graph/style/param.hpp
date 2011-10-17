// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_PARAM_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_PARAM_HPP 1

#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>

#include <list>

#include <iostream>

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
          namespace param
          {
            template<typename Key, typename Value>
            class traits
            {
            public:
              typedef boost::function< boost::optional<Value> (Key)
                                     > predicate_type;
              typedef boost::unordered_map<Key, boost::optional<Value>
                                          > cache_type;
              typedef std::list<predicate_type> predicates_type;
            };

            template<typename Key, typename Value>
            class store
            {
            private:
              typedef traits<Key, Value> super;

            public:
              void push (typename super::predicate_type const& predicate)
              {
                _cache.clear();
                _predicates.push_back (predicate);
              }

              boost::optional<Value> get (const Key& key)
              {
                const typename super::cache_type::const_iterator
                  cache_pos (_cache.find (key));

                if (cache_pos != _cache.end())
                  {
                    std::cerr << "CACHE: " << key << " -> " << cache_pos->second << std::endl;

                    return cache_pos->second;
                  }

                typename super::predicates_type::const_iterator
                  predicate (_predicates.begin());

                while (predicate != _predicates.end())
                  {
                    boost::optional<Value> value ((*predicate) (key));

                    if (value)
                      {
                        _cache.insert
                          ( typename super::cache_type::value_type ( key
                                                                   , value
                                                                   )
                          );

                        std::cerr << "RESULT: " << key << " -> " << *value << std::endl;

                        return value;
                      }

                    ++predicate;
                  }

                _cache.insert
                  (typename super::cache_type::value_type (key, boost::none));

                return boost::none;
              }

            private:
              typename super::cache_type _cache;
              typename super::predicates_type _predicates;
            };
          }
        }
      }
    }
  }
}

#endif
