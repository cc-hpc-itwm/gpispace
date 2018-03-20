// mirko.rahn@itwm.fraunhofer.de

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <drts/information_to_reattach.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/information_to_reattach.hpp>

#include <we/type/activity.hpp>
#include <we/type/value.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <sdpa/client.hpp>

#include <we/type/activity.hpp>
#include <we/type/copy.hpp>

#include <boost/format.hpp>

#include <iostream>

namespace gspc
{
  struct workflow::implementation
  {
    implementation (::we::type::activity_t activity)
      : _activity (activity)
    {}

    ::we::type::activity_t _activity;
  };

  workflow::workflow (boost::filesystem::path workflow)
    : _ (new implementation (::we::type::activity_t (workflow)))
  {}
  PIMPL_DTOR (workflow)

  workflow::workflow (workflow&& other)
    : _ (std::move (other._))
  {}

  void workflow::set_wait_for_output()
  {
    _->_activity.transition().set_property ({"drts", "wait_for_output"}, true);
  }
  std::string workflow::to_string() const
  {
    return _->_activity.to_string();
  }
  void workflow::add_input ( std::string const& port
                           , pnet::type::value::value_type const& value
                           )
  {
    _->_activity.add_input
      (_->_activity.transition().input_port_by_name (port), value);
  }

  static_assert ( std::is_same<job_id_t, sdpa::job_id_t>::value
                , "drts::job_id_t != sdpa::job_id_t"
                );

  struct client::implementation
  {
    implementation
      ( gspc::host_and_port_type const& orchestrator_endpoint
      , certificates_t const& certificates
      )
      : _client ( fhg::com::host_t (orchestrator_endpoint.host)
                , fhg::com::port_t (std::to_string (orchestrator_endpoint.port))
                , fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , certificates
                )
    {}

    sdpa::client::Client _client;
  };

  client::client (scoped_runtime_system const& drts, certificates_t const& certificates)
    : client (information_to_reattach (drts), certificates)
  {}
  client::client (scoped_runtime_system const& drts)
    : client (information_to_reattach (drts), boost::none)
  {}
  client::client
    ( information_to_reattach const& drts_info
    , certificates_t const& certificates
    )
    : _ (new implementation (drts_info._->endpoint(), certificates))
  {}
  PIMPL_DTOR (client)

  namespace
  {
    void put ( ::we::type::activity_t& activity
             , std::multimap< std::string
                            , pnet::type::value::value_type
                            > const& values_on_ports
             )
    {
      for (auto const& value_on_port : values_on_ports)
      {
        activity.add_input
          ( activity.transition().input_port_by_name (value_on_port.first)
          , value_on_port.second
          );
      }
    }
  }

  job_id_t client::submit
    ( class workflow const& workflow
    , std::multimap< std::string
                   , pnet::type::value::value_type
                   > const& values_on_ports
    )
  {
    put (workflow._->_activity, values_on_ports);

    return _->_client.submitJob (workflow._->_activity);
  }

  void client::put_token ( job_id_t job_id
                         , std::string place_name
                         , pnet::type::value::value_type value
                         )
  {
    _->_client.put_token (job_id, place_name, value);
  }

  namespace
  {
    void wait_for_terminal_state (job_id_t job_id, sdpa::client::Client& client)
    {
      sdpa::client::job_info_t job_info;

      sdpa::status::code const status
        (client.wait_for_terminal_state (job_id, job_info));

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

    ::we::type::activity_t wait_and_delete_job
      (job_id_t job_id, sdpa::client::Client& client)
    {
      wait_for_terminal_state (job_id, client);

      ::we::type::activity_t const result_activity
        (client.retrieveResults (job_id));

      client.deleteJob (job_id);

      return result_activity;
    }
  }

  void client::step (class workflow& workflow, unsigned long number_of_steps)
  {
    workflow._->_activity =
      copy (workflow._->_activity, std::string ("_STEP"), boost::none);

    for (unsigned long i (0); i < number_of_steps; i++)
    {
      put (workflow._->_activity, {{"_STEP", ::we::type::literal::control()}});
    }

    job_id_t const job_id (submit (workflow, {}));

    workflow._->_activity =
      copy_rev ( wait_and_delete_job (job_id, _->_client)
               , std::string ("_STEP")
               );
  }

  void client::break_after
    (class workflow& workflow, std::vector<std::string> transition_names)
  {
    workflow._->_activity =
      copy (workflow._->_activity, std::string ("_CONTROL"), transition_names);

    put (workflow._->_activity, {{"_CONTROL", ::we::type::literal::control()}});

    job_id_t const job_id (submit (workflow, {}));

    workflow._->_activity =
      copy_rev ( wait_and_delete_job (job_id, _->_client)
               , std::string ("_CONTROL")
               );
  }

  void client::wait (job_id_t job_id) const
  {
    wait_for_terminal_state (job_id, _->_client);
  }
  void client::cancel (job_id_t job_id) const
  {
    _->_client.cancelJob (job_id);
  }

  std::multimap<std::string, pnet::type::value::value_type>
    client::extract_result_and_forget_job (job_id_t job_id)
  {
    ::we::type::activity_t const result_activity
      (wait_and_delete_job (job_id, _->_client));

    std::multimap<std::string, pnet::type::value::value_type> result;

    for (auto const& value_on_port: result_activity.output())
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
    return _->_client.workflow_response (job_id, place_name, value);
  }
}
