// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/net.hpp>

#include <pnete/data/change_manager.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        net::net (const net_meta_base::id_type& id, internal_type* document)
          : net_meta_base (id, document)
        { }

        void net::add_transition
          (const boost::optional<QPointF>& position) const
        {
          change_manager().add_transition (*this, position);
        }
        void net::add_transition
          ( const xml::parse::id::ref::function& function
          , const boost::optional<QPointF>& position
          ) const
        {
          change_manager().add_transition (function, *this, position);
        }

        void net::add_place (const boost::optional<QPointF>& position) const
        {
          change_manager().add_place (*this, position);
        }

        void net::add_connection_with_implicit_place
          (const port& left, const port& right) const
        {
          change_manager().add_connection (left, right, *this);
        }

        void net::add_connection_or_association
          (const place& left, const port& right) const
        {
          change_manager().add_connection (left, right);
        }
      }
    }
  }
}
