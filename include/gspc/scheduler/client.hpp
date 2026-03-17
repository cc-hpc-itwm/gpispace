// Copyright (C) 2010,2012-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/SchedulerEvent.hpp>
#include <gspc/scheduler/job_states.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/value.hpp>

#include <gspc/com/channel.hpp>

#include <boost/asio/io_service.hpp>

#include <gspc/util/threadsafe_queue.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>


  namespace gspc::scheduler::client
  {
    struct job_info_t
    {
      std::string error_message;
    };

    class Client
    {
    public:
      Client ( gspc::com::host_t const& top_level_agent_host
             , gspc::com::port_t const& top_level_agent_port
             , std::unique_ptr<::boost::asio::io_service> peer_io_service
             , gspc::Certificates const& certificates
             );
      ~Client();
      Client (Client const&) = delete;
      Client (Client&&) = delete;
      Client& operator= (Client const&) = delete;
      Client& operator= (Client&&) = delete;

      job_id_t submitJob (gspc::we::type::Activity);
      void cancelJob (job_id_t const&);
      void deleteJob (job_id_t const&);
      void put_token
        (job_id_t, std::string place_name, gspc::pnet::type::value::value_type);
      gspc::pnet::type::value::value_type workflow_response
        (job_id_t, std::string place_name, gspc::pnet::type::value::value_type);

      gspc::scheduler::status::code wait_for_terminal_state (job_id_t, job_info_t&);

      gspc::we::type::Activity result (gspc::scheduler::job_id_t const&) const;

    private:
      std::mutex _make_client_thread_safe;

      gspc::util::threadsafe_queue<gspc::scheduler::events::SchedulerEvent::Ptr> m_incoming_events;

      template<typename Expected, typename Sent>
        Expected send_and_wait_for_reply (Sent event);

      void handle_recv (gspc::com::peer_t::Received);

      bool _stopping {false};
      gspc::com::channel m_peer;
      std::unordered_map<gspc::scheduler::job_id_t, gspc::we::type::Activity> _job_results;
    };
  }
