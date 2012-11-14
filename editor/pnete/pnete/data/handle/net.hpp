// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_NET_HPP
#define _FHG_PNETE_DATA_HANDLE_NET_HPP 1

#include <pnete/data/handle/net.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/net.fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class net : public meta_base < ::xml::parse::id::ref::net
                                     , ::xml::parse::type::net_type
                                     >
        {
        public:
          net ( const meta_base::id_type& id
              , change_manager_t& change_manager
              );

          using meta_base::operator==;
        };
      }
    }
  }
}

#endif
