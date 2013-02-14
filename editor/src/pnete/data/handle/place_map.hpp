// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_MAP_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_MAP_HPP 1

#include <pnete/data/handle/place_map.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/place_map.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::place_map
                          , ::xml::parse::type::place_map_type
                          > place_map_meta_base;
        class place_map : public place_map_meta_base
        {
        public:
          place_map (const place_map_meta_base::id_type&, internal_type*);

          void remove() const;

          virtual void set_property ( const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          place resolved_real_place() const;
          port tunnel_port() const;

          using place_map_meta_base::operator==;
        };
      }
    }
  }
}

#endif
