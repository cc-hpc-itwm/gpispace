// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/port.hpp>

#include <pnete/data/change_manager.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/port.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        port::port ( const port_type& port
                   , const handle::function& function
                   , change_manager_t& change_manager
                   )
          : base (change_manager)
          , _port_id (port.id())
          , _function (function)
        { }

        port::port_type& port::operator()() const
        {
          const boost::optional<port_type&> port
            (function()().port_by_id_ref (_port_id));
          if (!port)
          {
            throw fhg::util::backtracing_exception
              ("INVALID HANDLE: port id not found");
          }
          return *port;
        }

        const handle::function& port::function() const
        {
          return _function;
        }

        bool port::operator== (const port& other) const
        {
          return _port_id == other._port_id && _function == other._function;
        }

        const ::xml::parse::id::port& port::id() const
        {
          return _port_id;
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
      }
    }
  }
}
