// mirko.rahn@itwm.fraunhofer.de

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <boost/format.hpp>

#include <iostream>

namespace gspc
{
  static_assert ( std::is_same<job_id_t, sdpa::job_id_t>::value
                , "drts::job_id_t != sdpa::job_id_t"
                );

  client::client (scoped_runtime_system const& drts)
    : _client
      ("orchestrator", drts._kvs_host, std::to_string (drts._kvs_port))
  {}

  job_id_t client::submit
    ( boost::filesystem::path const& workflow
    , std::multimap< std::string
                   , pnet::type::value::value_type
                   > const& values_on_ports
    )
  {
    return submit (we::type::activity_t (workflow), values_on_ports);
  }

  job_id_t client::submit
    ( we::type::activity_t activity
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
      activity.add_input
        ( activity.transition().input_port_by_name (value_on_port.first)
        , value_on_port.second
        );
    }

    return _client.submitJob (activity.to_string());
  }

  void client::put_token ( job_id_t job_id
                         , std::string place_name
                         , pnet::type::value::value_type value
                         )
  {
    _client.put_token (job_id, place_name, value);
  }

  std::multimap<std::string, pnet::type::value::value_type>
    client::wait_and_extract (job_id_t job_id)
  {
    std::cerr << "waiting for job " << job_id << std::endl;

    sdpa::client::job_info_t job_info;

    sdpa::status::code const status
      (_client.wait_for_terminal_state (job_id, job_info));

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
      (_client.retrieveResults (job_id));

    _client.deleteJob (job_id);

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
