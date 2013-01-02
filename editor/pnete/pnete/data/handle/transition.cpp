// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/transition.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/transition.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        transition::transition ( const transition_meta_base::id_type& id
                               , change_manager_t& change_manager
                               )
          : transition_meta_base (id, change_manager)
        { }

        void transition::remove (const QObject* sender) const
        {
          change_manager().delete_transition (sender, *this);
        }

        void transition::set_name
          (const QObject* sender, const QString& name) const
        {
          change_manager().set_name (sender, *this, name);
        }

        void transition::set_property
          ( const QObject* sender
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void transition::move ( const QObject* sender
                              , const QPointF& position
                              ) const
        {
          change_manager().move_item (sender, *this, position);
        }

        void transition::no_undo_move ( const QObject* sender
                                      , const QPointF& position
                                      ) const
        {
          change_manager().no_undo_move_item (sender, *this, position);
        }

        net transition::parent() const
        {
          return net (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
