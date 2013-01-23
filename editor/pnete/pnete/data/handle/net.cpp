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

        void net::add_transition ( const QObject* sender
                                 , const boost::optional<QPointF>& position
                                 ) const
        {
          change_manager().add_transition (sender, *this, position);
        }
        void net::add_transition
          ( const QObject* origin
          , const xml::parse::id::ref::function& function
          , const boost::optional<QPointF>& position
          ) const
        {
          change_manager().add_transition (origin, function, *this, position);
        }

        void net::add_place ( const QObject* sender
                            , const boost::optional<QPointF>& position
                            ) const
        {
          change_manager().add_place (sender, *this, position);
        }

        void net::add_connection_with_implicit_place
          (const QObject* origin, const port& left, const port& right) const
        {
          change_manager().add_connection (origin, left, right, *this);
        }

        void net::add_connection_or_association
          (const QObject* origin, const place& left, const port& right) const
        {
          change_manager().add_connection (origin, left, right);
        }
      }
    }
  }
}
