#pragma once

#include <we/type/value.hpp>
#include <we/type/value/to_value.hpp>

#include <boost/function.hpp>

#include <boost/foreach.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>

#include <we/type/value/show.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        inline std::list<value_type>
        wrap (std::list<T> const& lT)
      {
        std::list<value_type> lv;

        BOOST_FOREACH (T const& x, lT)
        {
          lv.emplace_back (to_value (x));
        }

        return lv;
      }

      template<>
        inline std::list<value_type>
        wrap<value_type> (std::list<value_type> const& lv)
      {
        return lv;
      }

      template<typename T>
        inline std::set<value_type>
        wrap (std::set<T> const& sT)
      {
        std::set<value_type> sv;

        BOOST_FOREACH (T const& x, sT)
        {
          sv.emplace (to_value (x));
        }

        return sv;
      }

      template<typename K, typename V>
        inline std::map<value_type, value_type>
        wrap (std::map<K, V> const& mkv)
      {
        std::map<value_type, value_type> mvv;

        BOOST_FOREACH
          (typename std::map<K BOOST_PP_COMMA() V>::value_type const& kv, mkv)
        {
          mvv.emplace ( to_value (kv.first)
                      , to_value (kv.second)
                      );
        }

        return mvv;
      }

      template<>
        inline std::map<value_type, value_type>
        wrap<value_type, value_type> (std::map< value_type
                                              , value_type
                                              > const& mvv
                                     )
      {
        return mvv;
      }

      template<>
        inline std::set<value_type>
        wrap<value_type> (std::set<value_type> const& lv)
      {
        return lv;
      }
    }
  }
}
