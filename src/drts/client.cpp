// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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

  workflow::workflow (boost::filesystem::path workflow_)
    : _ (new implementation (::we::type::activity_t (workflow_)))
  {}
  PIMPL_DTOR (workflow)

  workflow::workflow (workflow&& other)
    : _ (std::move (other._))
  {}

  void workflow::set_wait_for_output()
  {
    _->_activity.set_wait_for_output();
  }
  std::string workflow::to_string() const
  {
    return _->_activity.to_string();
  }

  static_assert ( std::is_same<job_id_t, sdpa::job_id_t>::value
                , "drts::job_id_t != sdpa::job_id_t"
                );

  struct client::implementation
  {
    implementation
      ( gspc::host_and_port_type const& top_level_agent_endpoint
      , Certificates const& certificates
      )
      : _client ( fhg::com::host_t (top_level_agent_endpoint.host)
                , fhg::com::port_t (std::to_string (top_level_agent_endpoint.port))
                , fhg::util::cxx14::make_unique<boost::asio::io_service>()
                , certificates
                )
    {}

    sdpa::client::Client _client;
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
                   , pnet::type::value::value_type
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

  std::multimap<std::string, pnet::type::value::value_type>
    client::extract_result_and_forget_job (job_id_t job_id)
  {
    return wait_and_delete_job (job_id, _->_client).result();
  }

  std::multimap<std::string, pnet::type::value::value_type>
    client::wait_and_extract (job_id_t job_id)
  {
    return extract_result_and_forget_job (job_id);
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
