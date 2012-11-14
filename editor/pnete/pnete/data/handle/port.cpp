// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/port.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/port.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        port::port ( const meta_base::id_type& id
                   , change_manager_t& change_manager
                   )
          : meta_base (id, change_manager)
        { }

        void port::set_property ( const QObject* sender
                                , const ::we::type::property::key_type& key
                                , const ::we::type::property::value_type& val
                                ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void port::move ( const QObject* sender
                        , const QPointF& position
                        ) const
        {
          change_manager().move_item (sender, *this, position);
        }

        function port::parent() const
        {
          return function (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
