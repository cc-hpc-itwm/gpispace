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
        typedef meta_base < ::xml::parse::id::ref::net
                          , ::xml::parse::type::net_type
                          > net_meta_base;
        class net : public net_meta_base
        {
        public:
          net ( const net_meta_base::id_type& id
              , change_manager_t& change_manager
              );

          void add_transition (const QObject* sender) const;
          void add_place (const QObject* sender) const;

          using net_meta_base::operator==;
        };
      }
    }
  }
}

#endif
