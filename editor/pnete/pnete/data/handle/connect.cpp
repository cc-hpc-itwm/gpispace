// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/connect.hpp>

#include <pnete/data/change_manager.hpp>

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

        void connect::is_read (const QObject* origin, const bool& s) const
        {
          change_manager().connection_is_read (origin, *this, s);
        }

        void connect::remove (const QObject* sender) const
        {
          change_manager().remove_connection (sender, *this);
        }

        void connect::set_property
          ( const QObject* sender
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }
      }
    }
  }
}
