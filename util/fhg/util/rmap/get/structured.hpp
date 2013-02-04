// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_GET_STRUCTURED_HPP
#define _FHG_UTIL_RMAP_GET_STRUCTURED_HPP

#include <fhg/util/rmap/traits.hpp>
#include <fhg/util/rmap/structured/type.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      namespace visitor
      {
        template<typename Key, typename Mapped>
        class get_structured : public boost::static_visitor
                       <boost::optional<const structured::type<Key, Mapped>&>
                       >
        {
        public:
          FHG_UTIL_RMAP_TRAITS();

          boost::optional<const structured_type&>
          operator() (const structured_type& s) const
          {
            return s;
          }
          boost::optional<const structured_type&>
          operator() (const mapped_type&) const
          {
            return boost::none;
          }
        };
      }

      template<typename Key, typename Mapped>
      boost::optional<const structured::type<Key, Mapped>&>
      get_structured (const typename traits<Key, Mapped>::variant_type& variant)
      {
        return boost::apply_visitor
          (visitor::get_structured<Key, Mapped>(), variant);
      }
    }
  }
}

#endif
