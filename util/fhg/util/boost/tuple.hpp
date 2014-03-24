// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_TUPLE_HPP
#define FHG_UTIL_BOOST_TUPLE_HPP

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

#include <functional>

namespace std
{
  namespace
  {
    template < class tuple_type
             , size_t index = boost::tuples::length<tuple_type>::value - 1
             >
      struct hash_value_impl
      {
        static void apply (size_t& seed, const tuple_type& tuple)
        {
          hash_value_impl<tuple_type, index - 1>::apply (seed, tuple);
          boost::hash_combine (seed, tuple.template get<index>());
        }
      };

    template <class tuple_type>
      struct hash_value_impl<tuple_type, 0>
    {
      static void apply (size_t& seed, const tuple_type& tuple)
      {
        boost::hash_combine (seed, tuple.template get<0>());
      }
    };
  }

  template<typename... Values>
    struct hash<boost::tuples::tuple<Values...>>
  {
    std::size_t operator() (const boost::tuples::tuple<Values...>& tuple) const
    {
      size_t seed = 0;
      hash_value_impl<boost::tuples::tuple<Values...>>::apply (seed, tuple);
      return seed;
    }
  };
}

#endif
