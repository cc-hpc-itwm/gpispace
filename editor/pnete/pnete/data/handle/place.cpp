// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place::place ( const meta_base::id_type& id
                     , change_manager_t& change_manager
                     )
          : meta_base (id, change_manager)
        { }

        void place::remove (const QObject* sender) const
        {
          change_manager().delete_place (sender, *this);
        }

        void place::set_property
          ( const QObject* sender
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void place::move ( const QObject* sender
                         , const QPointF& position
                         ) const
        {
          change_manager().move_item (sender, *this, position);
        }

        void place::no_undo_move ( const QObject* sender
                                 , const QPointF& position
                                 ) const
        {
          change_manager().no_undo_move_item (sender, *this, position);
        }

        net place::parent() const
        {
          return net (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
