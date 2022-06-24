// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>

#include <we/type/Activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value.hpp>

#include <fhgcom/channel.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace sdpa
{
  namespace client
  {
    struct job_info_t
    {
      std::string error_message;
    };

    class Client
    {
    public:
      Client ( fhg::com::host_t const& top_level_agent_host
             , fhg::com::port_t const& top_level_agent_port
             , std::unique_ptr<::boost::asio::io_service> peer_io_service
             , fhg::com::Certificates const& certificates
             );
      ~Client();
      Client (Client const&) = delete;
      Client (Client&&) = delete;
      Client& operator= (Client const&) = delete;
      Client& operator= (Client&&) = delete;

      job_id_t submitJob (we::type::Activity);
      void cancelJob (job_id_t const&);
      void deleteJob (job_id_t const&);
      void put_token
        (job_id_t, std::string place_name, pnet::type::value::value_type);
      pnet::type::value::value_type workflow_response
        (job_id_t, std::string place_name, pnet::type::value::value_type);

      sdpa::status::code wait_for_terminal_state (job_id_t, job_info_t&);

      we::type::Activity result (sdpa::job_id_t const&) const;

    private:
      std::mutex _make_client_thread_safe;

      fhg::util::threadsafe_queue<sdpa::events::SDPAEvent::Ptr> m_incoming_events;

      template<typename Expected, typename Sent>
        Expected send_and_wait_for_reply (Sent event);

      void handle_recv (fhg::com::peer_t::Received);

      bool _stopping {false};
      fhg::com::channel m_peer;
      std::unordered_map<sdpa::job_id_t, we::type::Activity> _job_results;
    };
  }
}
