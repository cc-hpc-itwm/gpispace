#include <drts/information_to_reattach.hpp>
#include <drts/private/information_to_reattach.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>

#include <we/type/value.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/join.hpp>

#include <boost/lexical_cast.hpp>

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
            + fhg::util::join (path.begin(), path.end(), ".")
            + ": expected type " + typeid (T).name()
            );
        }
      }
      else
      {
        throw std::logic_error
          ( "value did not contain anything at path: "
          + fhg::util::join (path.begin(), path.end(), ".")
          );
      }
    }

    gspc::host_and_port_type get_orchestrator_endpoint
      (pnet::type::value::value_type const& value)
    {
      return { get_or_throw<std::string> ({"orchestrator", "host"}, value)
             , static_cast<unsigned short> (get_or_throw<unsigned int> ({"orchestrator", "port"}, value))
             };
    }
  }

  information_to_reattach::implementation::implementation (std::string const& serialized_value)
    : _endpoint (get_orchestrator_endpoint (pnet::type::value::read (serialized_value)))
  {}

  information_to_reattach::implementation::implementation (host_and_port_type const& orchestrator)
    : _endpoint (orchestrator)
  {}

  std::string information_to_reattach::implementation::to_string () const
  {
    pnet::type::value::value_type serialized;
    pnet::type::value::poke ( std::list<std::string> {"orchestrator", "host"}
                            , serialized
                            , _endpoint.host
                            );
    pnet::type::value::poke ( std::list<std::string> {"orchestrator", "port"}
                            , serialized
                            , static_cast<unsigned int> (_endpoint.port)
                            );
    return boost::lexical_cast<std::string> (pnet::type::value::show (serialized));
  }

  gspc::host_and_port_type const&
  information_to_reattach::implementation::endpoint() const
  {
    return _endpoint;
  }

  information_to_reattach::information_to_reattach (scoped_runtime_system const& drts)
    : _ (new implementation
          ( gspc::host_and_port_type
              { drts._->_started_runtime_system._orchestrator_host
              , drts._->_started_runtime_system._orchestrator_port
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
