// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_WRAP_HPP
#define PNET_SRC_WE_TYPE_VALUE_WRAP_HPP

#include <we/type/value.hpp>
#include <we/type/value/to_value.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>


#include <we/type/value/show.hpp>
#include <iostream>

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
          lv.push_back (to_value (x));
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
          sv.insert (to_value (x));
        }

        return sv;
      }

      template<typename K, typename V>
        inline std::map<value_type, value_type>
        wrap (std::map<K, V> const& mkv)
      {
        std::map<value_type, value_type> mvv;

        typedef typename std::map<K, V>::value_type kv_type;

        BOOST_FOREACH (kv_type const& kv, mkv)
        {
          mvv.insert (std::make_pair ( to_value (kv.first)
                                     , to_value (kv.second)
                                     )
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

      template<typename T, typename C>
        class wrapped_comparator
      {
      public:
        wrapped_comparator ( C const& comparator
                           , boost::function<T (value_type const&)> const& cast
                           )
          : _comparator (comparator)
          , _cast (cast)
        {}
        bool operator() (value_type const& l, value_type const& r) const
        {
          return _comparator (_cast (l), _cast (r));
        }
      private:
        C const& _comparator;
        boost::function<T (value_type const&)> const& _cast;
      };

      template<typename T, typename C>
        inline std::set<value_type, wrapped_comparator<T, C> >
        wrap ( std::set<T, C> const& sT
             , boost::function<T (value_type const&)> const& cast
             )
      {
        std::set<value_type, wrapped_comparator<T, C> >
          sv (wrapped_comparator<T, C> (sT.key_comp(), cast));

        BOOST_FOREACH (T const& x, sT)
        {
          sv.insert (to_value (x));
        }

        return sv;
      }
    }
  }
}

#endif
