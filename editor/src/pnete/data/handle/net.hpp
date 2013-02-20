// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_NET_HPP
#define _FHG_PNETE_DATA_HANDLE_NET_HPP 1

#include <pnete/data/handle/net.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/net.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>

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
          net (const net_meta_base::id_type&, internal_type*);

          void add_transition (const boost::optional<QPointF>&) const;
          void add_transition ( const xml::parse::id::ref::function&
                              , const boost::optional<QPointF>&
                              ) const;
          void add_place (const boost::optional<QPointF>&) const;

          //! \todo Are these correct in net? They might fit more in
          //! transition or function. This would take detection of
          //! which type of connection is needed into handles / ui
          //! though, instead of doing it in change manager.
          void add_connection_with_implicit_place
            (const port&, const port&) const;
          void add_connection_or_association (const place&, const port&) const;

          using net_meta_base::operator==;
        };
      }
    }
  }
}

#endif
