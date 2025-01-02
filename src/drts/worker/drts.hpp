// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/Client.hpp>
#include <gspc/iml/SharedMemoryAllocation.hpp>

#include <fhgcom/channel.hpp>
#include <logging/stream_emitter.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <we/loader/loader.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <string>
#include <vector>

struct wfe_task_t;

class DRTSImpl final : public sdpa::events::EventHandler
{
public:
  class Job
  {
  public:
    enum state_t
    {
      PENDING
    , RUNNING
    , FINISHED
    , FAILED
    , CANCELED
    , CANCELED_DUE_TO_WORKER_SHUTDOWN
    };

    Job ( std::string const& jobid
        , we::type::Activity const& activity_
        , ::boost::optional<std::string> const& target_impl_
        , std::set<std::string> const& workers_
        )
      : id (jobid)
      , activity (activity_)
      , target_impl (target_impl_)
      , workers (workers_)
      , state (Job::PENDING)
      , result()
      , message ("")
    {}

    std::string const id;
    we::type::Activity activity;
    ::boost::optional<std::string> const target_impl;
    std::set<std::string> const workers;
    std::atomic<state_t> state;
    we::type::Activity result;
    std::string message;
  };

private:
  using map_of_jobs_t = std::map<std::string, std::shared_ptr<DRTSImpl::Job>>;

public:

  DRTSImpl
    ( std::function<void()> request_stop
    , std::unique_ptr<::boost::asio::io_service> peer_io_service
    , std::string const& kernel_name
    , unsigned short comm_port
    , iml::Client /*const*/* virtual_memory_socket
    , iml::SharedMemoryAllocation /*const*/* shared_memory
    , std::tuple<fhg::com::host_t, fhg::com::port_t> const& parent
    , std::vector<std::string> const& capability_names
    , std::vector<::boost::filesystem::path> const& library_path
    , fhg::logging::stream_emitter& log_emitter
    , gspc::Certificates const&
    );
  ~DRTSImpl() override;
  DRTSImpl (DRTSImpl const&) = delete;
  DRTSImpl (DRTSImpl&&) = delete;
  DRTSImpl& operator= (DRTSImpl const&) = delete;
  DRTSImpl& operator= (DRTSImpl&&) = delete;

  void handle_worker_registration_response
    (fhg::com::p2p::address_t const& source, const sdpa::events::worker_registration_response *e) override;
  void handleSubmitJobEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent *e) override;
  void handleCancelJobEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent *e) override;
  void handleJobFailedAckEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent *e) override;
  void handleJobFinishedAckEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent *e) override;

private:
  void event_thread();
  void job_execution_thread();

  void start_receiver();

  template<typename Event, typename... Args>
    void send_event_to_parent (Args&&... args);

  std::function<void()> _request_stop;

  bool m_shutting_down {false};

  std::string m_my_name;

  std::mutex _currently_executed_tasks_mutex;
  std::map<std::string, wfe_task_t *> _currently_executed_tasks;

  we::loader::loader m_loader;

  fhg::logging::stream_emitter& _log_emitter;
  void emit_gantt (wfe_task_t const&, sdpa::daemon::NotificationEvent::state_t);

  iml::Client /*const*/* _virtual_memory_api;
  iml::SharedMemoryAllocation /*const*/* _shared_memory;

  fhg::util::interruptible_threadsafe_queue<sdpa::events::SDPAEvent::Ptr
                                           > m_event_queue;

  std::mutex m_job_map_mutex;

  map_of_jobs_t m_jobs;

  fhg::util::interruptible_threadsafe_queue<std::shared_ptr<DRTSImpl::Job>>
    m_pending_jobs;

  fhg::com::channel _peer;

  ::boost::strict_scoped_thread<> m_event_thread;
  decltype (m_event_queue)::interrupt_on_scope_exit _interrupt_event_thread;
  ::boost::strict_scoped_thread<> m_execution_thread;
  decltype (m_pending_jobs)::interrupt_on_scope_exit _interrupt_execution_thread;

  std::promise<void> _registration_response;
  bool _registered {false};

  struct mark_remaining_tasks_as_canceled_helper
  {
    mark_remaining_tasks_as_canceled_helper
      ( std::mutex& currently_executed_tasks_mutex
      , std::map<std::string, wfe_task_t *>& currently_executed_tasks
      , std::mutex& jobs_guard
      , map_of_jobs_t& jobs
      )
        : _currently_executed_tasks_mutex {currently_executed_tasks_mutex}
        , _currently_executed_tasks {currently_executed_tasks}
        , _jobs_guard {jobs_guard}
        , _jobs {jobs}
    {}
    ~mark_remaining_tasks_as_canceled_helper();
    mark_remaining_tasks_as_canceled_helper (mark_remaining_tasks_as_canceled_helper const&) = delete;
    mark_remaining_tasks_as_canceled_helper (mark_remaining_tasks_as_canceled_helper&&) = delete;
    mark_remaining_tasks_as_canceled_helper& operator= (mark_remaining_tasks_as_canceled_helper const&) = delete;
    mark_remaining_tasks_as_canceled_helper& operator= (mark_remaining_tasks_as_canceled_helper&&) = delete;

    std::mutex& _currently_executed_tasks_mutex;
    std::map<std::string, wfe_task_t *>& _currently_executed_tasks;
    std::mutex& _jobs_guard;
    map_of_jobs_t& _jobs;
  } _mark_remaining_tasks_as_canceled_helper
    = { _currently_executed_tasks_mutex, _currently_executed_tasks
      , m_job_map_mutex, m_jobs
      };
};
