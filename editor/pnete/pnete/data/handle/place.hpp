// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_HPP 1

#include <pnete/data/handle/place.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/internal.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/place.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class place
        {
        private:
          typedef ::xml::parse::type::place_type place_type;

        public:
          place ( const place_type& place
                , const handle::net& net
                , change_manager_t& change_manager
                );

          place_type operator()() const;

          const handle::net& net() const;

          bool operator== (const place& other) const;

          const ::xml::parse::id::place& id() const;

          void remove (const QObject* sender) const;

        private:
          change_manager_t& change_manager() const;

          ::xml::parse::id::place _place_id;
          handle::net _net;
          change_manager_t& _change_manager;
        };
      }
    }
  }
}

#endif
