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
        net::net ( const net_meta_base::id_type& id
                 , change_manager_t& change_manager
                 )
          : net_meta_base (id, change_manager)
        { }
      }
    }
  }
}
