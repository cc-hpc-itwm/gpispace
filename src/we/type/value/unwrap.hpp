#pragma once

#include <we/type/value.hpp>
#include <we/type/value/from_value.hpp>

#include <boost/foreach.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template <typename T>
      inline std::list<T> unwrap (std::list<value_type> const &lv)
      {
        std::list<T> lt;

        BOOST_FOREACH (value_type const &v, lv)
        {
          lt.emplace_back (from_value<T> (v));
        }

        return lt;
      }

      template <typename T>
      inline std::set<T> unwrap (std::set<value_type> const &sv)
      {
        std::set<T> st;

        BOOST_FOREACH (value_type const &v, sv)
        {
          st.emplace (from_value<T> (v));
        }

        return st;
      }

      template <typename K, typename V>
      inline std::map<K, V> unwrap (std::map<value_type, value_type> const &mvv)
      {
        std::map<K, V> mkv;

        BOOST_FOREACH
          ( std::map<value_type BOOST_PP_COMMA() value_type>::value_type const&
            kv
          , mvv
          )
        {
          mkv.emplace ( from_value<K> (kv.first)
                      , from_value<V> (kv.second)
                      );
        }

        return mkv;
      }
    }
  }
}
