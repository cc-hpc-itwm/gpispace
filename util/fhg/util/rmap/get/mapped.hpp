// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_GET_MAPPED_HPP
#define _FHG_UTIL_RMAP_GET_MAPPED_HPP

#include <fhg/util/rmap/traits.hpp>
#include <fhg/util/rmap/structured/type.hpp>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      namespace visitor
      {
        template<typename Key, typename Mapped>
        class get_mapped : public boost::static_visitor
                         <typename traits<Key, Mapped>::query_result_type>
        {
        public:
          FHG_UTIL_RMAP_TRAITS();

          query_result_type operator() (const structured_type&) const
          {
            return boost::none;
          }
          query_result_type operator() (const mapped_type& m) const
          {
            return m;
          }
        };
      }

      template<typename Key, typename Mapped>
      typename traits<Key, Mapped>::query_result_type
      get_mapped (const typename traits<Key, Mapped>::variant_type& variant)
      {
        return boost::apply_visitor
          (visitor::get_mapped<Key, Mapped>(), variant);
      }
    }
  }
}

#endif
