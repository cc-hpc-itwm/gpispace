// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/port.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/place.hpp>

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
        port::port (const port_meta_base::id_type& id, internal_type* document)
          : port_meta_base (id, document)
        { }

        void port::remove (const QObject* sender) const
        {
          change_manager().delete_port (sender, *this);
        }

        void port::set_name (const QObject* sender, const QString& name) const
        {
          change_manager().set_name (sender, *this, name);
        }

        void port::set_type (const QObject* sender, const QString& type) const
        {
          change_manager().set_type (sender, *this, type);
        }

        void port::remove_place_association (const QObject* sender) const
        {
          change_manager().set_place_association (sender, *this, boost::none);
        }

        bool port::is_input() const
        {
          return direction_is (we::type::PORT_IN);
        }
        bool port::is_output() const
        {
          return direction_is (we::type::PORT_OUT);
        }
        bool port::is_tunnel() const
        {
          return direction_is (we::type::PORT_TUNNEL);
        }
        bool port::direction_is (const we::type::PortDirection& direction) const
        {
          return get().direction() == direction;
        }

        bool port::is_connectable (const port& other) const
        {
          return get().parent() && other.get().parent()
            && ( get().parent()->id() == other.get().parent()->id()
               ? get().direction() != other.get().direction()
               : get().direction() == other.get().direction()
               );
        }

        bool port::is_connected() const
        {
          return get().place;
        }
        place port::connected_place() const
        {
          return place (*get().resolved_place(), document());
        }

        bool port::can_rename_to (const QString& name) const
        {
          return get().name() == name.toStdString()
            || !get().parent()
            || !get().parent()->ports().has
                 (std::make_pair (name.toStdString(), get().direction()));
        }

        void port::set_property ( const QObject* sender
                                , const ::we::type::property::key_type& key
                                , const ::we::type::property::value_type& val
                                ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void port::move ( const QObject* sender
                        , const QPointF& position
                        , const bool outer
                        ) const
        {
          change_manager().move_item (sender, *this, position, outer);
        }

        void port::no_undo_move ( const QObject* sender
                                , const QPointF& position
                                ) const
        {
          change_manager().no_undo_move_item (sender, *this, position);
        }

        bool port::parent_is (const function& function) const
        {
          return get().parent() && get().parent()->id() == function.id();
        }

        function port::parent() const
        {
          return function (get().parent()->make_reference_id(), document());
        }
      }
    }
  }
}
