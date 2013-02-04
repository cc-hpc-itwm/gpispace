// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_TYPE_HPP
#define _FHG_UTIL_RMAP_TYPE_HPP

#include <fhg/util/rmap/type.fwd.hpp>

#include <fhg/util/rmap/traits.hpp>
#include <fhg/util/rmap/structured/construct.hpp>
#include <fhg/util/rmap/structured/type.hpp>
#include <fhg/util/rmap/get/mapped.hpp>
#include <fhg/util/rmap/get/structured.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/foreach.hpp>

#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      template<typename Key, typename Mapped>
      class type
      {
      public:
        FHG_UTIL_RMAP_TRAITS();

        type()
          : _variant()
        {}

        template<typename T>
        type (const T& x)
          : _variant (x)
        {}

        const_ref_mapped_type bind ( const keys_type& keys
                                   , const_ref_mapped_type mapped
                                   )
        {
          return bind (keys.begin(), keys.end(), mapped, _variant);
        }
        const_ref_mapped_type bind ( const key_type& key
                                   , const_ref_mapped_type mapped
                                   )
        {
          return boost::get<const_ref_mapped_type>
            (structured (_variant)[key] = mapped);
        }
        query_result_type value (const keys_type& keys) const
        {
          return value (keys.begin(), keys.end(), _variant);
        }
        query_result_type value (const key_type& key) const
        {
          boost::optional<const structured_type&>
            s (get_structured<Key, Mapped> (_variant));

          if (s)
          {
            const boost::optional<const variant_type&> slot (s->get (key));

            if (slot)
            {
              return get_mapped<Key, Mapped> (*slot);
            }
          }

          return boost::none;
        }

      private:
        variant_type _variant;

        typedef typename keys_type::const_iterator keys_iterator_type;

        query_result_type value ( keys_iterator_type pos
                                , const keys_iterator_type& end
                                , const variant_type& variant
                                ) const
        {
          if (pos == end)
          {
            return get_mapped<Key, Mapped> (variant);
          }
          else
          {
            boost::optional<const structured_type&>
              s (get_structured<Key, Mapped> (variant));

            if (s)
            {
              const boost::optional<const variant_type&> slot (s->get (*pos));

              if (slot)
              {
                ++pos;

                return value (pos, end, *slot);
              }
            }
          }

          return boost::none;
        }

        structured_type& structured (variant_type& v)
        {
          return boost::get<structured_type&>
            (structured::construct<Key, Mapped> (v));
        }

        const_ref_mapped_type bind ( keys_iterator_type pos
                                   , const keys_iterator_type& end
                                   , const_ref_mapped_type mapped
                                   , variant_type& variant
                                   )
        {
          if (pos == end)
          {
            return boost::get<const_ref_mapped_type>(variant = mapped);
          }
          else
          {
            const key_type& key (*pos); ++pos;

            return bind (pos, end, mapped, structured(variant)[key]);
          }
        }

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP (_variant);
        }

        template<typename K, typename M>
        friend std::ostream& operator<< (std::ostream&, const type<K, M>&);
      };
    }
  }
}

#endif
