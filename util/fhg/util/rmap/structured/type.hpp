// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_STRUCTURED_TYPE_HPP
#define _FHG_UTIL_RMAP_STRUCTURED_TYPE_HPP

#include <fhg/util/rmap/structured/type.fwd.hpp>
#include <fhg/util/rmap/traits.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      namespace structured
      {
        template<typename Key, typename Mapped>
        class type
        {
        public:
          FHG_UTIL_RMAP_TRAITS();

          variant_type& operator[] (const key_type& key)
          {
            return _map[key];
          }
          boost::optional<const variant_type&> get (const key_type& key) const
          {
            const typename map_type::const_iterator pos (_map.find (key));

            if (pos != _map.end())
            {
              return pos->second;
            }

            return boost::none;
          }

        private:
          map_type _map;

          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive& ar, const unsigned int)
          {
            ar & BOOST_SERIALIZATION_NVP (_map);
          }
        };
      }
    }
  }
}

#endif
