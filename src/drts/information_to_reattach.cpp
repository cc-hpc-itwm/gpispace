#include <drts/information_to_reattach.hpp>
#include <drts/private/information_to_reattach.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>

#include <we/type/value.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include <list>
#include <utility>

namespace gspc
{
  namespace
  {
    template <typename T>
    T const& get_or_throw (std::list<std::string> const& path, pnet::type::value::value_type const& value)
    {
      boost::optional<pnet::type::value::value_type const&> const v
        (pnet::type::value::peek (path, value));
      if (v)
      {
        try
        {
          return boost::get<T> (*v);
        }
        catch (boost::bad_get const& ex)
        {
          throw std::logic_error
            ( "value did not had correct type at path: "
            + pnet::type::value::path::join (path)
            + ": expected type " + typeid (T).name()
            );
        }
      }
      else
      {
        throw std::logic_error
          ( "value did not contain anything at path: "
          + pnet::type::value::path::join (path)
          );
      }
    }

    gspc::host_and_port_type get_top_level_agent_endpoint
      (pnet::type::value::value_type const& value)
    {
      return { get_or_throw<std::string> ({"top_level_agent", "host"}, value)
             , static_cast<unsigned short> (get_or_throw<unsigned int> ({"top_level_agent", "port"}, value))
             };
    }
  }

  information_to_reattach::implementation::implementation (std::string const& serialized_value)
    : _endpoint (get_top_level_agent_endpoint (pnet::type::value::from_string (serialized_value)))
  {}

  information_to_reattach::implementation::implementation (host_and_port_type const& top_level_agent)
    : _endpoint (top_level_agent)
  {}

  std::string information_to_reattach::implementation::to_string () const
  {
    pnet::type::value::value_type serialized;
    pnet::type::value::poke ( std::list<std::string> {"top_level_agent", "host"}
                            , serialized
                            , _endpoint.host
                            );
    pnet::type::value::poke ( std::list<std::string> {"top_level_agent", "port"}
                            , serialized
                            , static_cast<unsigned int> (_endpoint.port)
                            );
    return pnet::type::value::to_string (serialized);
  }

  gspc::host_and_port_type const&
  information_to_reattach::implementation::endpoint() const
  {
    return _endpoint;
  }

  information_to_reattach::information_to_reattach (scoped_runtime_system const& drts)
    : _ (new implementation
          ( gspc::host_and_port_type
              { drts._->_started_runtime_system._top_level_agent_host
              , drts._->_started_runtime_system._top_level_agent_port
              }
          )
        )
  {}
  information_to_reattach::information_to_reattach (std::string const& drts_info)
    : _ (new implementation (drts_info))
  {}
  PIMPL_DTOR (information_to_reattach)
  std::string information_to_reattach::to_string () const
  {
    return _->to_string();
  }
}
