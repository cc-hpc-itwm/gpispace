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
        net::net ( const net_meta_base::id_type& id
                 , change_manager_t& change_manager
                 )
          : net_meta_base (id, change_manager)
        { }

        void net::add_transition (const QObject* sender) const
        {
          change_manager().add_transition (sender, *this);
        }
        void net::add_place (const QObject* sender) const
        {
          change_manager().add_place (sender, *this);
        }
      }
    }
  }
}
