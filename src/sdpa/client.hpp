// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
