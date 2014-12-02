// mirko.rahn@itwm.fraunhofer.de

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <gpi-space/pc/client/api.hpp>

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
  workflow::~workflow()
  {
    delete _;
    _ = nullptr;
  }

  void workflow::set_wait_for_output()
  {
    _->_activity.transition().set_property ({"drts", "wait_for_output"}, true);
  }

  static_assert ( std::is_same<job_id_t, sdpa::job_id_t>::value
                , "drts::job_id_t != sdpa::job_id_t"
                );

  struct client::implementation
  {
    implementation (scoped_runtime_system const& drts)
      : _client ( fhg::com::host_t (drts._orchestrator_host)
                , fhg::com::port_t (std::to_string (drts._orchestrator_port))
                , _peer_io_service
                , _kvs_client_io_service
                , drts._kvs_host, std::to_string (drts._kvs_port)
                )
    {}

    boost::asio::io_service _peer_io_service;
    boost::asio::io_service _kvs_client_io_service;
    sdpa::client::Client _client;
  };

  client::client (scoped_runtime_system const& drts)
    : _ (new implementation (drts))
  {}
  client::~client()
  {
    delete _;
    _ = nullptr;
  }

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

  std::multimap<std::string, pnet::type::value::value_type>
    client::wait_and_extract (job_id_t job_id)
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
}
