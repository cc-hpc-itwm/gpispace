// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/connect.hpp>

#include <pnete/data/change_manager.hpp>

#include <xml/parse/type/connect.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        connect::connect ( const connect_meta_base::id_type& id
                         , change_manager_t& change_manager
                         )
          : connect_meta_base (id, change_manager)
        { }

        void connect::set_property
          ( const QObject* sender
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }
      }
    }
  }
}
