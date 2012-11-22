// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/function.hpp>

#include <pnete/data/change_manager.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        function::function ( const function_meta_base::id_type& id
                           , change_manager_t& change_manager
                           )
          : function_meta_base (id, change_manager)
        { }

        void function::set_property ( const QObject* sender
                                    , const ::we::type::property::key_type& key
                                    , const ::we::type::property::value_type& val
                                    ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void function::set_name (const QObject* sender, const QString& name)
        {
          change_manager().set_function_name (sender, *this, name);
        }
      }
    }
  }
}
