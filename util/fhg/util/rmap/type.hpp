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

        typedef structured::type<Key, Mapped> structured_type;
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

      template<typename Key, typename Mapped>
      std::ostream&
      write_to ( std::ostream&
               , const typename traits<Key, Mapped>::variant_type&
               , unsigned int
               );

      template<typename Key, typename Mapped>
      std::ostream& operator<< (std::ostream& os, const type<Key, Mapped>& x)
      {
        return write_to<Key, Mapped> (os, x._variant, 0);
      }

      namespace visitor
      {
        template<typename Key, typename Mapped>
        class writer : public boost::static_visitor<std::ostream&>
        {
        public:
          FHG_UTIL_RMAP_TRAITS();

          writer (std::ostream& os, unsigned int level)
            : _os (os)
            , _level (level)
          {}

          typedef structured::type<Key, Mapped> structured_type;

          std::ostream& operator() (const mapped_type& m) const
          {
            add_header();
            return _os << m << std::endl;
          }
          std::ostream& operator() (const structured_type& s) const
          {
            typedef std::pair<key_type, variant_type> kv_type;

            BOOST_FOREACH (const kv_type& kv, s.map())
            {
              add_header();
              _os << kv.first << ":" << std::endl;
              write_to<Key, Mapped> (_os, kv.second, _level + 1);
            }

            return _os;
          }

        private:
          std::ostream& _os;
          unsigned int _level;

          void add_header() const
          {
            for (unsigned int i (0); i < _level; ++i)
            {
              _os << " | ";
            }
          }
        };
      }

      template<typename Key, typename Mapped>
      std::ostream&
      write_to ( std::ostream& os
               , const typename traits<Key, Mapped>::variant_type& variant
               , unsigned int level
               )
      {
        return boost::apply_visitor
          (visitor::writer<Key, Mapped> (os, level), variant);
      }
    }
  }
}

#endif
