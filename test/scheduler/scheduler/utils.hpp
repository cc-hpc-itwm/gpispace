// Copyright (C) 2013-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/client.hpp>
#include <gspc/scheduler/daemon/Agent.hpp>
#include <test/scheduler/NetworkStrategy.hpp>

#include <gspc/com/peer.hpp>

#include <gspc/logging/stdout_sink.hpp>

#include <gspc/util/threadsafe_queue.hpp>

#include <gspc/we/type/Activity.hpp>

#include <optional>
#include <boost/test/unit_test.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>



    namespace boost::test_tools::tt_detail
    {
      template<> struct print_log_value<gspc::scheduler::Capability>
      {
        void operator() (std::ostream&, gspc::scheduler::Capability const&) const;
      };
    }



namespace utils
{
  std::string random_peer_name();

  //! \todo unify with test/layer
  gspc::we::type::Activity module_call (std::string name);

  gspc::we::type::Activity module_call();

  gspc::we::type::Activity module_call
    (std::string name, bool might_use_multiple_workers);

  gspc::we::type::Activity module_call_with_max_num_retries
    (unsigned long maximum_number_of_retries);

  gspc::we::type::Activity net_with_one_child_requiring_workers
    (unsigned long count);

  gspc::we::type::Activity net_with_one_child_requiring_workers_and_num_retries
    (unsigned long count, unsigned long maximum_number_of_retries);

  gspc::we::type::Activity net_with_two_children_requiring_n_workers
    (unsigned long n);

  struct log_to_stdout
  {
    gspc::logging::stdout_sink& sink();
    log_to_stdout (gspc::scheduler::daemon::Agent& component);
  };

  class basic_drts_component;

  struct agent
  {
    agent (gspc::Certificates const&);

    agent() = delete;
    agent (agent const&) = delete;
    agent (agent&&) = delete;
    agent& operator= (agent const&) = delete;
    agent& operator= (agent&&) = delete;
    ~agent() = default;

    gspc::scheduler::daemon::Agent _;
    log_to_stdout _log_to_stdout = {_};

    std::string name() const;
    gspc::com::host_t host() const;
    gspc::com::port_t port() const;
  };

  using CapabilityNames = std::unordered_set<std::string>;

  // \todo Only used by Restart* tests. Is persistent worker names
  // actually relevant there? If not, remove!
  struct reused_component_name
  {
    //! Only construct with a result from component.name() or
    //! random_peer_name()!
    explicit reused_component_name (std::string name)
      : _name (name)
    {}
    std::string _name;
  };

  class basic_drts_component_no_logic : gspc::scheduler::events::EventHandler
  {
  public:
    basic_drts_component_no_logic (gspc::Certificates const&);
    basic_drts_component_no_logic
      (reused_component_name, gspc::Certificates const&);

    std::string name() const;
    gspc::com::host_t host() const;
    gspc::com::port_t port() const;

  private:
    gspc::util::interruptible_threadsafe_queue
      <std::pair<gspc::com::p2p::address_t, gspc::scheduler::events::SchedulerEvent::Ptr>>
        _event_queue;

  protected:
    std::string _name;
    gspc::scheduler::test::NetworkStrategy _network;

    void event_thread_fun();

    struct event_thread
    {
      event_thread (basic_drts_component_no_logic&);

      basic_drts_component_no_logic& _component;
      ::boost::strict_scoped_thread<> _event_thread;
      decltype (basic_drts_component_no_logic::_event_queue)::interrupt_on_scope_exit
        _interrupt_thread;
    };
  };

  class basic_drts_component : public basic_drts_component_no_logic
  {
  public:
    basic_drts_component (gspc::Certificates const&);
    basic_drts_component ( agent const& parent
                         , gspc::scheduler::Capabilities
                         , gspc::Certificates const&
                         );
    basic_drts_component ( agent const& parent
                         , CapabilityNames
                         , gspc::Certificates const&
                         );
    basic_drts_component ( reused_component_name
                         , agent const& parent
                         , gspc::Certificates const&
                         );
    ~basic_drts_component() override;
    basic_drts_component (basic_drts_component const&) = delete;
    basic_drts_component& operator= (basic_drts_component const&) = delete;
    basic_drts_component (basic_drts_component&&) = delete;
    basic_drts_component& operator= (basic_drts_component&&) = delete;

    void handle_worker_registration_response
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::worker_registration_response const*
      ) override;

    void handleWorkerRegistrationEvent
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::WorkerRegistrationEvent const*
      ) override;

    void handleErrorEvent
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::ErrorEvent const*
      ) override;

    void wait_for_workers_to_shutdown();

    void wait_for_registration();

  protected:
    std::optional<gspc::com::p2p::address_t> _parent;
    std::unordered_set<gspc::com::p2p::address_t> _accepted_workers;
    gspc::util::threadsafe_queue<std::optional<std::exception_ptr>> _registration_responses;

    struct event_thread_and_worker_join
      : basic_drts_component_no_logic::event_thread
    {
      event_thread_and_worker_join (basic_drts_component&);
      ~event_thread_and_worker_join();
      event_thread_and_worker_join (event_thread_and_worker_join const&) = delete;
      event_thread_and_worker_join& operator= (event_thread_and_worker_join const&) = delete;
      event_thread_and_worker_join (event_thread_and_worker_join&&) = delete;
      event_thread_and_worker_join& operator= (event_thread_and_worker_join&&) = delete;

      basic_drts_component& _component_logic;
    };

  private:
    std::mutex _mutex_workers_shutdown;
    std::condition_variable _cond_workers_shutdown;
  };

  namespace no_thread
  {
    class basic_drts_worker : public basic_drts_component
    {
    public:
      basic_drts_worker
        ( agent const& parent
        , gspc::Certificates const&
        );
      basic_drts_worker
        ( agent const& parent
        , gspc::scheduler::Capabilities
        , gspc::Certificates const&
        );
      basic_drts_worker
        ( reused_component_name
        , agent const& parent
        , gspc::Certificates const&
        );
    };

    class fake_drts_worker_notifying_module_call_submission
      : public basic_drts_worker
    {
    public:
      fake_drts_worker_notifying_module_call_submission
        ( std::function<void (std::string)> announce_job
        , agent const& parent
        , gspc::Certificates const&
        );

      fake_drts_worker_notifying_module_call_submission
        ( reused_component_name name
        , std::function<void (std::string)>
        , agent const&
        , gspc::Certificates const&
        );

      void handleSubmitJobEvent
        ( gspc::com::p2p::address_t const&
        , gspc::scheduler::events::SubmitJobEvent const*
        ) override;
      void handleJobFinishedAckEvent
        ( gspc::com::p2p::address_t const&
        , gspc::scheduler::events::JobFinishedAckEvent const*
        ) override;

      std::string add_job ( gspc::we::type::Activity const& activity
                          , gspc::scheduler::job_id_t const&
                          , gspc::com::p2p::address_t const& owner
                          );

      void announce_job (std::string const& name);

    protected:
      struct job_t
      {
        gspc::scheduler::job_id_t _id;
        gspc::com::p2p::address_t _owner;
        gspc::we::type::Activity _activity;
      };
      std::map<std::string, job_t> _jobs;
      void delete_job (gspc::scheduler::job_id_t const& job_id);

    private:
      std::function<void (std::string)> _announce_job;
    };

    class fake_drts_worker_waiting_for_finished_ack
      : public no_thread::fake_drts_worker_notifying_module_call_submission
    {
    public:
      fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , agent const& parent
        , gspc::Certificates const&
        );

      fake_drts_worker_waiting_for_finished_ack
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& parent
        , gspc::Certificates const&
        );

      void handleJobFinishedAckEvent
        ( gspc::com::p2p::address_t const&
        , gspc::scheduler::events::JobFinishedAckEvent const*
        ) override;

      // Ignore cancel events - this worker type is designed to just finish
      // jobs and wait for acks. Cancel events may arrive during shutdown
      // races and should not cause test failures.
      void handleCancelJobEvent
        ( gspc::com::p2p::address_t const&
        , gspc::scheduler::events::CancelJobEvent const*
        ) override
      {}

      void finish_and_wait_for_ack (std::string name);

    private:
      gspc::util::threadsafe_queue<std::string> _finished_ack;
    };
  }

  struct basic_drts_worker final : public no_thread::basic_drts_worker
  {
    basic_drts_worker ( agent const& parent
                      , gspc::Certificates const&
                      );
    basic_drts_worker ( agent const& parent
                      , gspc::scheduler::Capabilities
                      , gspc::Certificates const&
                      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_notifying_module_call_submission final
    : public no_thread::fake_drts_worker_notifying_module_call_submission
  {
    fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const&
      );

    fake_drts_worker_notifying_module_call_submission
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const&
      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_waiting_for_finished_ack final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
    fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const&
      );

    fake_drts_worker_waiting_for_finished_ack
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& parent
      , gspc::Certificates const&
      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent
      , gspc::Certificates const&
      );

    void handleCancelJobEvent
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::CancelJobEvent const*
      ) override;

    void canceled (std::string job_id);

  private:
    std::function<void (std::string)> _announce_cancel;
    std::mutex _cancels_mutex;
    std::map<std::string, gspc::com::p2p::address_t> _cancels;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel_but_never_replying final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& parent
      , gspc::Certificates const&
      );

    void handleCancelJobEvent
      ( gspc::com::p2p::address_t const&
      , gspc::scheduler::events::CancelJobEvent const*
      ) override;

  private:
    std::function<void (std::string)> _announce_cancel;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct client
  {
    client (agent const&, gspc::Certificates const&);

    client() = delete;
    client (client const&) = delete;
    client (client&&) = delete;
    client& operator= (client const&) = delete;
    client& operator= (client&&) = delete;
    ~client() = default;

    gspc::scheduler::job_id_t submit_job (gspc::we::type::Activity);

    gspc::scheduler::status::code wait_for_terminal_state (gspc::scheduler::job_id_t const&);
    gspc::scheduler::status::code wait_for_terminal_state
      (gspc::scheduler::job_id_t const&, gspc::scheduler::client::job_info_t& job_info);

    gspc::scheduler::status::code wait_for_terminal_state_and_cleanup
      (gspc::scheduler::job_id_t const&);

    void delete_job (gspc::scheduler::job_id_t const&);

    void cancel_job (gspc::scheduler::job_id_t const&);

    gspc::we::type::Activity result (gspc::scheduler::job_id_t const&) const;

    gspc::scheduler::client::Client _;
  };

  std::string get_scheduler_type (bool);
}
