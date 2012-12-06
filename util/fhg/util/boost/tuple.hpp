// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_TUPLE_HPP
#define FHG_UTIL_BOOST_TUPLE_HPP

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;
      namespace detail
      {
        template < class tuple_type
                 , size_t index = tuples::length<tuple_type>::value - 1
                 >
          struct hash_value_impl
        {
          static void apply (size_t& seed, const tuple_type& tuple)
          {
            hash_value_impl<tuple_type, index - 1>::apply (seed, tuple);
            hash_combine (seed, tuple.get<index>());
          }
        };

        template <class tuple_type>
          struct hash_value_impl<tuple_type,0>
        {
          static void apply (size_t& seed, const tuple_type& tuple)
          {
            hash_combine (seed, tuple.get<0>());
          }
        };
      }
    }
  }
}

namespace boost
{
  namespace tuples
  {
    //! \note Needs to be here to allow ADL with the scope of tuple_type.
    template <class tuple_type>
      static inline size_t hash_value (const tuple_type& tuple)
    {
      size_t seed = 0;
      fhg::util::boost::detail::hash_value_impl<tuple_type>::apply (seed, tuple);
      return seed;
    }
  }
}

#endif
