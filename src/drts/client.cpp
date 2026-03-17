// Copyright (C) 2014-2015,2018-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>

#include <gspc/drts/information_to_reattach.hpp>
#include <gspc/drts/private/information_to_reattach.hpp>
#include <gspc/drts/private/pimpl.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/value.hpp>

#include <gspc/scheduler/client.hpp>

#include <fmt/core.h>
#include <iostream>

namespace gspc
{
  struct workflow::implementation
  {
    implementation (::gspc::we::type::Activity activity)
      : _activity (activity)
    {}

    ::gspc::we::type::Activity _activity;
  };

  workflow::workflow (std::filesystem::path workflow_)
    : _ (new implementation (::gspc::we::type::Activity (workflow_)))
  {}
  PIMPL_DTOR (workflow)

  void workflow::set_wait_for_output()
  {
    _->_activity.set_wait_for_output();
  }
  std::string workflow::to_string() const
  {
    return _->_activity.to_string();
  }

  workflow::workflow (workflow&&) noexcept = default;
  workflow& workflow::operator= (workflow&&) noexcept = default;

  static_assert ( std::is_same<job_id_t, gspc::scheduler::job_id_t>::value
                , "drts::job_id_t != gspc::scheduler::job_id_t"
                );

  struct client::implementation
  {
    implementation
      ( gspc::host_and_port_type const& top_level_agent_endpoint
      , Certificates const& certificates
      )
      : _client ( gspc::com::host_t (top_level_agent_endpoint.host)
                , gspc::com::port_t (top_level_agent_endpoint.port)
                , std::make_unique<::boost::asio::io_service>()
                , certificates
                )
    {}

    gspc::scheduler::client::Client _client;
  };

  client::client (scoped_runtime_system const& drts, Certificates const& certificates)
    : client (information_to_reattach (drts), certificates)
  {}
  client::client
    ( information_to_reattach const& drts_info
    , Certificates const& certificates
    )
    : _ (new implementation (drts_info._->endpoint(), certificates))
  {}
  PIMPL_DTOR (client)

  job_id_t client::submit
    ( class workflow const& workflow
    , std::multimap< std::string
                   , gspc::pnet::type::value::value_type
                   > const& values_on_ports
    )
  {
    for (auto const& value_on_port : values_on_ports)
    {
      workflow._->_activity.add_input (value_on_port.first, value_on_port.second);
    }

    return _->_client.submitJob (workflow._->_activity);
  }

  void client::put_token ( job_id_t job_id
                         , std::string place_name
                         , gspc::pnet::type::value::value_type value
                         )
  {
    _->_client.put_token (job_id, place_name, value);
  }

  namespace
  {
    void wait_for_terminal_state (job_id_t job_id, gspc::scheduler::client::Client& client)
    {
      gspc::scheduler::client::job_info_t job_info;

      gspc::scheduler::status::code const status
        (client.wait_for_terminal_state (job_id, job_info));

      if (gspc::scheduler::status::FAILED == status)
      {
        //! \todo decorate the exception with the most progressed activity
        throw std::runtime_error
          { fmt::format
            ( "Job {}: failed: error-message := {}"
            , job_id
            , job_info.error_message
            )
          };
      }
    }

    ::gspc::we::type::Activity wait_and_delete_job
      (job_id_t job_id, gspc::scheduler::client::Client& client)
    {
      wait_for_terminal_state (job_id, client);

      auto const result_activity (client.result (job_id));

      client.deleteJob (job_id);

      return result_activity;
    }
  }

  void client::wait (job_id_t job_id) const
  {
    wait_for_terminal_state (job_id, _->_client);
  }
  void client::cancel (job_id_t job_id) const
  {
    _->_client.cancelJob (job_id);
  }

  std::multimap<std::string, gspc::pnet::type::value::value_type>
    client::extract_result_and_forget_job (job_id_t job_id)
  {
    return wait_and_delete_job (job_id, _->_client).result();
  }

  std::multimap<std::string, gspc::pnet::type::value::value_type>
    client::wait_and_extract (job_id_t job_id)
  {
    return extract_result_and_forget_job (job_id);
  }

  gspc::pnet::type::value::value_type client::synchronous_workflow_response
    ( job_id_t job_id
    , std::string place_name
    , gspc::pnet::type::value::value_type value
    )
  {
    return _->_client.workflow_response (job_id, place_name, value);
  }
}
