// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/net.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        net::net ( const ::xml::parse::id::ref::net& id
                 , change_manager_t& change_manager
                 )
          : base (change_manager)
          , _id (id)
        { }

        const ::xml::parse::type::net_type& net::get() const
        {
          return _id.get();
        }
        ::xml::parse::type::net_type& net::get_ref() const
        {
          return _id.get_ref();
        }

        bool net::operator== (const net& other) const
        {
          return _id == other._id;
        }

        const ::xml::parse::id::ref::net& net::id() const
        {
          return _id;
        }
      }
    }
  }
}
