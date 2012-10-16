// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_NET_HPP
#define _FHG_PNETE_DATA_HANDLE_NET_HPP 1

#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/util/id_type.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class net
        {
        private:
          typedef ::xml::parse::type::net_type net_type;

        public:
          net (const net_type& net);

          net_type& operator()() const;

          bool operator== (const net& other) const;

          const ::xml::parse::id::net& id() const;

        private:
          ::xml::parse::id::net _net_id;
          //! \todo Remove the reference.
          net_type& _net;
        };
      }
    }
  }
}

#endif
