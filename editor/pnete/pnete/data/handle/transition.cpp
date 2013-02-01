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
                               , internal_type* document
                               )
          : transition_meta_base (id, document)
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

        bool transition::can_rename_to (const QString& name) const
        {
          return !( get().parent()
                  && get().parent()->has_transition (name.toStdString())
                  );
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
                              , const bool outer
                              ) const
        {
          change_manager().move_item (sender, *this, position, outer);
        }

        void transition::no_undo_move ( const QObject* sender
                                      , const QPointF& position
                                      ) const
        {
          change_manager().no_undo_move_item (sender, *this, position);
        }

        bool transition::parent_is (const net& net) const
        {
          return get().parent() && get().parent()->id() == net.id();
        }

        net transition::parent() const
        {
          return net (get().parent()->make_reference_id(), document());
        }
      }
    }
  }
}
