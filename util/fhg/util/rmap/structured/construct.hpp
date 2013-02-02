// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_STRUCTURED_CONSTRUCT_HPP
#define _FHG_UTIL_RMAP_STRUCTURED_CONSTRUCT_HPP

#include <fhg/util/rmap/traits.hpp>
#include <fhg/util/rmap/structured/type.hpp>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      namespace structured
      {
        namespace visitor
        {
          template<typename Key, typename Mapped>
          class construct : public boost::static_visitor
                          <typename traits<Key, Mapped>::variant_type&>
          {
          public:
            FHG_UTIL_RMAP_TRAITS();

            typedef type<Key, Mapped> structured_type;

            construct (variant_type& variant)
              : _variant (variant)
            {}
            variant_type& operator() (structured_type& s) const
            {
              return _variant;
            }
            variant_type& operator() (mapped_type&) const
            {
              return _variant = structured_type();
            }

          private:
            variant_type& _variant;
          };
        }

        template<typename Key, typename Mapped>
        typename traits<Key, Mapped>::variant_type& construct
        (typename traits<Key, Mapped>::variant_type& variant)
        {
          return boost::apply_visitor
            (visitor::construct<Key, Mapped> (variant), variant);
        }
      }
    }
  }
}

#endif
