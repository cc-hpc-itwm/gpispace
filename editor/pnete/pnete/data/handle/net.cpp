// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        net::net (const net_type& net)
          : _net_id (net.id())
          , _net (const_cast<net_type&> (net))
        { }

        net::net_type& net::operator()() const
        {
          //! \todo Get net by id.
          return _net;
        }

        bool net::operator== (const net& other) const
        {
          return _net_id == other._net_id;
        }

        const ::xml::parse::id::net& net::id() const
        {
          return _net_id;
        }
      }
    }
  }
}

