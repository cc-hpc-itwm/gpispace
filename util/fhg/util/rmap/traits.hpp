// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_RMAP_TRAITS_HPP
#define _FHG_UTIL_RMAP_TRAITS_HPP

#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <fhg/util/rmap/type.fwd.hpp>
#include <fhg/util/rmap/structured/type.fwd.hpp>

#include <map>
#include <list>

namespace fhg
{
  namespace util
  {
    namespace rmap
    {
      template<typename Key, typename Mapped>
      class traits
      {
      public:
        typedef Key key_type;
        typedef Mapped mapped_type;
        typedef typename std::list<key_type> keys_type;
        typedef typename boost::add_reference<const Mapped>::type
                                                      const_ref_mapped_type;
        typedef typename boost::add_reference<Mapped>::type ref_mapped_type;
        typedef typename structured::type<Key, Mapped> structured_type;
        typedef boost::variant< Mapped
                              , boost::recursive_wrapper<structured_type>
                              > variant_type;
        typedef std::map<key_type, variant_type> map_type;
        typedef std::pair<key_type, variant_type> kv_type;
        typedef boost::optional<const_ref_mapped_type> query_result_type;
        typedef type<Key, Mapped> rmap_type;
      };

#define FHG_UTIL_RMAP_TRAITS()                                          \
      typedef traits<Key,Mapped> traits_type;                           \
                                                                        \
      typedef typename traits_type::key_type              key_type;     \
      typedef typename traits_type::mapped_type           mapped_type;  \
      typedef typename traits_type::kv_type               kv_type;      \
      typedef typename traits_type::structured_type       structured_type; \
      typedef typename traits_type::keys_type             keys_type;    \
      typedef typename traits_type::const_ref_mapped_type const_ref_mapped_type; \
      typedef typename traits_type::ref_mapped_type       ref_mapped_type; \
      typedef typename traits_type::variant_type          variant_type; \
      typedef typename traits_type::map_type              map_type;     \
      typedef typename traits_type::query_result_type     query_result_type
    }
  }
}

#endif
