// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/connect.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>

#include <xml/parse/type/connect.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        connect::connect ( const connect_meta_base::id_type& id
                         , internal_type* document
                         )
          : connect_meta_base (id, document)
        { }

        bool connect::is_in() const
        {
          return petri_net::edge::is_PT (get().direction());
        }
        bool connect::is_out() const
        {
          return !is_in();
        }
        bool connect::is_read() const
        {
          return petri_net::edge::is_pt_read (get().direction());
        }

        void connect::is_read (const bool& s) const
        {
          change_manager().connection_is_read (*this, s);
        }

        port connect::resolved_port() const
        {
          return port (*get().resolved_port(), document());
        }
        place connect::resolved_place() const
        {
          return place (*get().resolved_place(), document());
        }

        void connect::remove() const
        {
          change_manager().remove_connection (*this);
        }

        void connect::set_property
          ( const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (*this, key, val);
        }
      }
    }
  }
}
