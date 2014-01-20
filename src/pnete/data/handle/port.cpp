// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/port.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/place.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/transition.hpp>

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

        void port::remove() const
        {
          change_manager().delete_port (*this);
        }

        void port::set_name (const QString& name) const
        {
          change_manager().set_name (*this, name);
        }

        void port::set_type (const QString& type) const
        {
          change_manager().set_type (*this, type);
        }
        QString port::type() const
        {
          return QString::fromStdString (get().type());
        }

        void port::remove_place_association() const
        {
          change_manager().set_place_association (*this, boost::none);
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

        namespace
        {
          // not_null_equal
          template<typename opt>
            bool nne (const opt& lhs, const opt& rhs)
          {
            return lhs && rhs && lhs == rhs;
          }

          we::type::PortDirection fake_dir (const handle::port& port)
          {
            if (port.get().direction() == we::type::PORT_TUNNEL)
            {
              const boost::optional<std::string> tunnel_direction
                (port.get().properties().get
                  ("fhg.pnete.tunnel.direction")
                );

              return !tunnel_direction ? we::type::PORT_TUNNEL
                :  *tunnel_direction == "out" ? we::type::PORT_OUT
                : *tunnel_direction == "in" ? we::type::PORT_IN
                : throw std::runtime_error
                  ("bad fhg.pnete.tunnel.direction (neither 'in' nor 'out')");
            }
            else
            {
              return port.get().direction();
            }
          }
        }

        bool port::is_connectable (const port& other) const
        {
          if (type() != other.type())
          {
            return false;
          }

          const boost::optional<xml::parse::id::function> this_function
            ( get().parent()
            ? get().parent()->id()
            : boost::optional<xml::parse::id::function> (boost::none)
            );
          const boost::optional<xml::parse::id::function> other_function
            ( other.get().parent()
            ? other.get().parent()->id()
            : boost::optional<xml::parse::id::function> (boost::none)
            );

          const boost::optional<xml::parse::id::function> this_net_function
            ( this_function
            && get().parent()->parent_transition()
            && get().parent()->parent_transition()->get().parent()
            && get().parent()->parent_transition()->get().parent()->parent()
            ? get().parent()->parent_transition()->get().parent()->parent()->id()
            : boost::optional<xml::parse::id::function> (boost::none)
            );
          const boost::optional<xml::parse::id::function> other_net_function
            ( other_function
            && other.get().parent()->parent_transition()
            && other.get().parent()->parent_transition()->get().parent()
            && other.get().parent()->parent_transition()->get().parent()->parent()
            ? other.get().parent()->parent_transition()
            ->get().parent()->parent()->id()
            : boost::optional<xml::parse::id::function> (boost::none)
            );

          const we::type::PortDirection this_dir (fake_dir (*this));
          const we::type::PortDirection other_dir (fake_dir (other));

          const bool one_is_tunnel ( this_dir == we::type::PORT_TUNNEL
                                   || other_dir == we::type::PORT_TUNNEL
                                   );
          const bool same_direction (this_dir == other_dir);

          return ( ( nne (this_function, other_function)
                   || nne (other_net_function, this_net_function)
                   )
                 && (!same_direction || one_is_tunnel)
                 )
              || ( ( nne (this_function, other_net_function)
                   || nne (this_net_function, other_function)
                   )
                 && (same_direction || one_is_tunnel)
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

        void port::set_property ( const ::we::type::property::key_type& key
                                , const ::we::type::property::value_type& val
                                ) const
        {
          change_manager().set_property (*this, key, val);
        }

        void port::move (const QPointF& position, const bool outer) const
        {
          change_manager().move_item (*this, position, outer);
        }

        void port::no_undo_move (const QPointF& position) const
        {
          change_manager().no_undo_move_item (*this, position);
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
