// mirko.rahn@itwm.fraunhofer.de

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <drts/information_to_reattach.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/information_to_reattach.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/serialize.hpp>

#include <network/server.hpp>

#include <rpc/server.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/boost/asio/ip/address.hpp>

#include <sdpa/client.hpp>

#include <we/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>

#include <boost/format.hpp>

#include <iostream>

namespace gspc
{
  struct workflow::implementation
  {
    implementation (we::type::activity_t activity)
      : _activity (activity)
    {}

    we::type::activity_t _activity;
  };

  workflow::workflow (boost::filesystem::path workflow)
    : _ (new implementation (we::type::activity_t (workflow)))
  {}
  PIMPL_DTOR (workflow)

  void workflow::set_wait_for_output()
  {
    _->_activity.transition().set_property ({"drts", "wait_for_output"}, true);
  }

  static_assert ( std::is_same<job_id_t, sdpa::job_id_t>::value
                , "drts::job_id_t != sdpa::job_id_t"
                );

  struct client::implementation
  {
    implementation (gspc::host_and_port_type const& orchestrator_endpoint)
      : _client ( fhg::com::host_t (orchestrator_endpoint.host)
                , fhg::com::port_t (std::to_string (orchestrator_endpoint.port))
                , fhg::util::cxx14::make_unique<boost::asio::io_service>()
                )
    {}

    sdpa::client::Client _client;
  };

  client::client (scoped_runtime_system const& drts)
    : client (information_to_reattach (drts))
  {}
  client::client (information_to_reattach const& drts_info)
    : _ (new implementation (drts_info._->endpoint()))
  {}
  PIMPL_DTOR (client)

  job_id_t client::submit
    ( class workflow const& workflow
    , std::multimap< std::string
                   , pnet::type::value::value_type
                   > const& values_on_ports
    )
  {
    for ( std::pair< std::string
                   , pnet::type::value::value_type
                   > const& value_on_port
        : values_on_ports
        )
    {
      workflow._->_activity.add_input
        ( workflow._->_activity.transition()
        . input_port_by_name (value_on_port.first)
        , value_on_port.second
        );
    }

    return _->_client.submitJob (workflow._->_activity.to_string());
  }

  void client::put_token ( job_id_t job_id
                         , std::string place_name
                         , pnet::type::value::value_type value
                         )
  {
    _->_client.put_token (job_id, place_name, value);
  }

  void client::wait (job_id_t job_id) const
  {
    std::cerr << "waiting for job " << job_id << std::endl;

    sdpa::client::job_info_t job_info;

    sdpa::status::code const status
      (_->_client.wait_for_terminal_state (job_id, job_info));

    if (sdpa::status::FAILED == status)
    {
      //! \todo decorate the exception with the most progressed activity
      throw std::runtime_error
        (( boost::format ("Job %1%: failed: error-message := %2%")
         % job_id
         % job_info.error_message
         ).str()
        );
    }
  }

  std::multimap<std::string, pnet::type::value::value_type>
    client::extract_result_and_forget_job (job_id_t job_id)
  {
    we::type::activity_t const result_activity
      (_->_client.retrieveResults (job_id));

    _->_client.deleteJob (job_id);

    std::multimap<std::string, pnet::type::value::value_type> result;

    for ( std::pair<pnet::type::value::value_type, we::port_id_type>
           const& value_on_port
        : result_activity.output()
        )
    {
      result.emplace
        ( result_activity.transition().ports_output()
        . at (value_on_port.second).name()
        , value_on_port.first
        );
    }

    return result;
  }

  pnet::type::value::value_type client::synchronous_workflow_response
    ( job_id_t job_id
    , std::string place_name
    , pnet::type::value::value_type value
    )
  {
    fhg::rpc::service_dispatcher service_dispatcher
      {fhg::util::serialization::exception::serialization_functions()};

    fhg::util::thread::event<pnet::type::value::value_type> result;

    fhg::rpc::service_handler<void (pnet::type::value::value_type)>
      const service_handler_set_result
        ( service_dispatcher
        , "set_result"
        , [&result] (pnet::type::value::value_type value)
          {
            result.notify (value);
          }
        );

    boost::asio::io_service io_service;
    std::unique_ptr<fhg::network::connection_type> connection;
    fhg::util::thread::event<void> disconnected;

    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
      ( boost::asio::ip::tcp::endpoint()
      , io_service
      , [] (fhg::network::buffer_type buf) { return buf; }
      , [] (fhg::network::buffer_type buf) { return buf; }
      , [&service_dispatcher] ( fhg::network::connection_type* connection
                              , fhg::network::buffer_type message
                              )
        {
          service_dispatcher.dispatch (connection, message);
        }
      , [&connection, &disconnected] (fhg::network::connection_type*)
        {
          connection.reset();
          disconnected.notify();
        }
      , [&connection] (std::unique_ptr<fhg::network::connection_type> c)
        {
          if (!!connection)
          {
            throw std::logic_error
              ("workflow_response: got a second connection");
          }
          std::swap (connection, c);
        }
      );

    const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      io_service_thread ([&io_service]() { io_service.run(); });

    struct stop_io_service_on_scope_exit
    {
      ~stop_io_service_on_scope_exit()
      {
        _io_service.stop();
      }
      boost::asio::io_service& _io_service;
    } stop_io_service_on_scope_exit {io_service};

    pnet::type::value::value_type value_and_endpoint;
    pnet::type::value::poke ("value", value_and_endpoint, value);
    pnet::type::value::poke ( "address"
                            , value_and_endpoint
                            , fhg::util::connectable_to_address_string
                              (acceptor.local_endpoint().address())
                            );
    pnet::type::value::poke
      ( "port"
      , value_and_endpoint
      , static_cast<unsigned int> (acceptor.local_endpoint().port())
      );

    put_token (job_id, place_name, value_and_endpoint);

    disconnected.wait();

    return result.wait();
  }
}
