#ifndef PNET_SRC_WE_TYPE_VALUE_UNWRAP_HPP
#define PNET_SRC_WE_TYPE_VALUE_UNWRAP_HPP

#include <we/type/value.hpp>
#include <boost/foreach.hpp>

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
          lt.push_back (boost::get<T>(v));
        }

        return lt;
      }

      template <typename T>
      inline std::set<T> unwrap (std::set<value_type> const &sv)
      {
        std::set<T> st;

        BOOST_FOREACH (value_type const &v, sv)
        {
          st.insert (boost::get<T>(v));
        }

        return st;
      }

      template <typename K, typename V>
      inline std::map<K, V> unwrap (std::map<value_type, value_type> const &mvv)
      {
        std::map<K, V> mkv;

        typedef std::map<value_type, value_type>::value_type kv_type;

        BOOST_FOREACH (kv_type const &kv, mvv)
        {
          mkv.insert (std::make_pair ( boost::get<K>(kv.first)
                                     , boost::get<V>(kv.second)
                                     )
                     );
        }

        return mkv;
      }
    }
  }
}

#endif
