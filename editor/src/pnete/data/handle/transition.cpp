// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/transition.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>
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

        void transition::remove() const
        {
          change_manager().delete_transition (*this);
        }

        void transition::set_name (const QString& name) const
        {
          change_manager().set_name (*this, name);
        }

        bool transition::can_rename_to (const QString& name) const
        {
          return get().name() == name.toStdString()
            || !get().parent()
            || !get().parent()->has_transition (name.toStdString());
        }

        void transition::set_property
          ( const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (*this, key, val);
        }

        void transition::move (const QPointF& position, const bool outer) const
        {
          change_manager().move_item (*this, position, outer);
        }

        void transition::no_undo_move (const QPointF& position) const
        {
          change_manager().no_undo_move_item (*this, position);
        }

        handle::function transition::function() const
        {
          return handle::function (get().resolved_function(), document());
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