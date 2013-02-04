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
          : _node()
        {}

        template<typename T>
        type (const T& x)
          : _node (x)
        {}

        const_ref_mapped_type bind ( const keys_type& keys
                                   , const_ref_mapped_type mapped
                                   )
        {
          return bind (keys.begin(), keys.end(), mapped, _node);
        }
        const_ref_mapped_type bind ( const key_type& key
                                   , const_ref_mapped_type mapped
                                   )
        {
          return boost::get<const_ref_mapped_type>
            (structured (_node)[key] = mapped);
        }
        query_result_type value (const keys_type& keys) const
        {
          return value (keys.begin(), keys.end(), _node);
        }
        query_result_type value (const key_type& key) const
        {
          boost::optional<const structured_type&>
            s (get_structured<Key, Mapped> (_node));

          if (s)
          {
            const boost::optional<const node_type&> slot (s->get (key));

            if (slot)
            {
              return get_mapped<Key, Mapped> (*slot);
            }
          }

          return boost::none;
        }

      private:
        node_type _node;

        typedef typename keys_type::const_iterator keys_iterator_type;

        query_result_type value ( keys_iterator_type pos
                                , const keys_iterator_type& end
                                , const node_type& node
                                ) const
        {
          if (pos == end)
          {
            return get_mapped<Key, Mapped> (node);
          }
          else
          {
            boost::optional<const structured_type&>
              s (get_structured<Key, Mapped> (node));

            if (s)
            {
              const boost::optional<const node_type&> slot (s->get (*pos));

              if (slot)
              {
                ++pos;

                return value (pos, end, *slot);
              }
            }
          }

          return boost::none;
        }

        structured_type& structured (node_type& node)
        {
          return boost::get<structured_type&>
            (structured::construct<Key, Mapped> (node));
        }

        const_ref_mapped_type bind ( keys_iterator_type pos
                                   , const keys_iterator_type& end
                                   , const_ref_mapped_type mapped
                                   , node_type& node
                                   )
        {
          if (pos == end)
          {
            return boost::get<const_ref_mapped_type>(node = mapped);
          }
          else
          {
            const key_type& key (*pos); ++pos;

            return bind (pos, end, mapped, structured(node)[key]);
          }
        }

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP (_node);
        }

        template<typename K, typename M>
        friend std::ostream& operator<< (std::ostream&, const type<K, M>&);
      };
    }
  }
}

#endif
