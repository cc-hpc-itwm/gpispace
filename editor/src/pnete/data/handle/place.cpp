// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

#include <fhg/util/read_bool.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place::place (const place_meta_base::id_type& id, internal_type* doc)
          : place_meta_base (id, doc)
        { }

        void place::remove() const
        {
          change_manager().delete_place (*this);
        }

        void place::set_name (const QString& name) const
        {
          change_manager().set_name (*this, name);
        }

        void place::set_type (const QString& type) const
        {
          change_manager().set_type (*this, type);
        }

        bool place::can_rename_to (const QString& name) const
        {
          return get().name() == name.toStdString()
            || !get().parent()
            || !get().parent()->has_place (name.toStdString());
        }

        void place::set_property
          ( const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (*this, key, val);
        }

        void place::move ( const QPointF& position
                         , const bool outer
                         ) const
        {
          change_manager().move_item (*this, position, outer);
        }

        void place::no_undo_move (const QPointF& position) const
        {
          change_manager().no_undo_move_item (*this, position);
        }

        bool place::is_implicit() const
        {
          return fhg::util::read_bool
            ( get().properties().get_with_default
              ("fhg.pnete.is_implicit_place", "false")
            );
        }

        void place::make_explicit() const
        {
          change_manager().make_explicit (*this);
        }

        bool place::is_virtual() const
        {
          return get().is_virtual();
        }

        void place::make_virtual() const
        {
          change_manager().make_virtual (*this);
        }
        void place::make_real() const
        {
          change_manager().make_real (*this);
        }

        bool place::parent_is (const net& net) const
        {
          return get().parent() && get().parent()->id() == net.id();
        }

        net place::parent() const
        {
          return net (get().parent()->make_reference_id(), document());
        }
      }
    }
  }
}
