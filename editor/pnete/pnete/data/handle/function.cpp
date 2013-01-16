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
                           , internal_type* document
                           )
          : function_meta_base (id, document)
        { }

        void function::set_property ( const QObject* sender
                                    , const ::we::type::property::key_type& key
                                    , const ::we::type::property::value_type& val
                                    ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void function::set_name (const QObject* sender, const QString& name) const
        {
          change_manager().set_function_name (sender, *this, name);
        }

        void function::add_port ( const QObject* origin
                                , const we::type::PortDirection& direction
                                , const boost::optional<QPointF>& position
                                ) const
        {
          change_manager().add_port (origin, *this, direction, position);
        }
      }
    }
  }
}
