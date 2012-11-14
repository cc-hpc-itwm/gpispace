// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_NET_HPP
#define _FHG_PNETE_DATA_HANDLE_NET_HPP 1

#include <pnete/data/handle/net.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/base.hpp>

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
        class net : public base
        {
        public:
          net ( const ::xml::parse::id::ref::net& id
              , change_manager_t& change_manager
              );

          const ::xml::parse::type::net_type& get() const;
          ::xml::parse::type::net_type& get_ref() const;

          bool operator== (const net& other) const;

          const ::xml::parse::id::ref::net& id() const;

        private:
          ::xml::parse::id::ref::net _id;
        };
      }
    }
  }
}

#endif
