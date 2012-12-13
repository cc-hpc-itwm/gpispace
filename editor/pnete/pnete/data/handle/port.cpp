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
        port::port ( const port_meta_base::id_type& id
                   , change_manager_t& change_manager
                   )
          : port_meta_base (id, change_manager)
        { }

        void port::remove (const QObject* sender) const
        {
          change_manager().delete_port (sender, *this);
        }

        void port::set_type (const QObject* sender, const QString& type) const
        {
          change_manager().set_type (sender, *this, type);
        }

        void port::remove_place_association (const QObject* sender) const
        {
          change_manager().set_place_association (sender, *this, boost::none);
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
                        ) const
        {
          change_manager().move_item (sender, *this, position);
        }

        void port::no_undo_move ( const QObject* sender
                                , const QPointF& position
                                ) const
        {
          change_manager().no_undo_move_item (sender, *this, position);
        }

        function port::parent() const
        {
          return function (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
