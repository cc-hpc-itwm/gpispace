// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/worker/drts.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/worker_registration_response.hpp>

#include <we/loader/exceptions.hpp>
#include <we/loader/module_call.hpp>
#include <we/type/Activity.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <cstdlib>
#include <random>
#include <stdexcept>

struct wfe_task_t
{
  enum state_t
  {
    PENDING
  , CANCELED
  , CANCELED_DUE_TO_WORKER_SHUTDOWN
  , FINISHED
  , FAILED
  };

  std::string id;
  state_t state {PENDING};
  we::type::Activity activity;
  ::boost::optional<std::string> const target_impl;
  drts::worker::context context;

  wfe_task_t ( std::string id_
             , we::type::Activity const& activity_
             , ::boost::optional<std::string> const& target_impl_
             , std::string worker_name
             , std::set<std::string> workers
             , fhg::logging::stream_emitter& logger
             )
    : id (id_)
    , activity (activity_)
    , target_impl (target_impl_)
    , context
      (drts::worker::context_constructor (worker_name, workers, logger))
  {}
};

DRTSImpl::mark_remaining_tasks_as_canceled_helper::~mark_remaining_tasks_as_canceled_helper()
{
  std::lock_guard<std::mutex> const currently_executed_tasks_lock
    (_currently_executed_tasks_mutex);
  std::lock_guard<std::mutex> const jobs_lock (_jobs_guard);

  for (auto& [_ignore, task] : _currently_executed_tasks)
  {
    task->state = wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN;
    task->context.module_call_do_cancel();
  }

  for (auto& [_ignore, job] : _jobs)
  {
    job->state = Job::state_t::CANCELED_DUE_TO_WORKER_SHUTDOWN;
  }
}

DRTSImpl::DRTSImpl
    ( std::function<void()> request_stop
    , std::unique_ptr<::boost::asio::io_service> peer_io_service
    , std::string const& kernel_name
    , unsigned short comm_port
    , iml::Client /*const*/* virtual_memory_api
    , iml::SharedMemoryAllocation /*const*/* shared_memory
    , std::tuple<fhg::com::host_t, fhg::com::port_t> const& parent
    , std::vector<std::string> const& capability_names
    , std::vector<::boost::filesystem::path> const& library_path
    , fhg::logging::stream_emitter& log_emitter
    , gspc::Certificates const& certificates
    )
  : _request_stop (request_stop)
  , m_my_name (kernel_name)
  , _currently_executed_tasks()
  , m_loader ({library_path.begin(), library_path.end()})
  , _log_emitter (log_emitter)
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
  , _peer ( std::move (peer_io_service)
          , fhg::com::port_t {comm_port}
          , certificates
          , std::get<0> (parent)
          , std::get<1> (parent)
          )
  , m_event_thread (&DRTSImpl::event_thread, this)
  , _interrupt_event_thread (m_event_queue)
  , m_execution_thread (&DRTSImpl::job_execution_thread, this)
  , _interrupt_execution_thread (m_pending_jobs)
{
  start_receiver();

  std::set<sdpa::Capability> capabilities;
  for (std::string const& cpb_name : capability_names)
  {
    capabilities.emplace (cpb_name);
  }

  send_event_to_parent<sdpa::events::WorkerRegistrationEvent>
    ( m_my_name
    , capabilities
    ,
    #if GSPC_WITH_IML
      (_shared_memory != nullptr) ? _shared_memory->size() : 0
    #else
      0
    #endif
    , fhg::util::hostname()
    );

  _registration_response.get_future().wait();

  _registered = true;
}

DRTSImpl::~DRTSImpl()
{
  m_shutting_down = true;
}

void DRTSImpl::handle_worker_registration_response
  ( fhg::com::p2p::address_t const&
  , sdpa::events::worker_registration_response const* response
  )
{
  try
  {
    response->get();

    _registration_response.set_value();
  }
  catch (...)
  {
    _registration_response.set_exception (std::current_exception());
  }
}

void DRTSImpl::handleSubmitJobEvent
  (fhg::com::p2p::address_t const&, const sdpa::events::SubmitJobEvent *e)
{
  if (!e->job_id())
  {
    throw std::runtime_error ("Received job with an unspecified job id");
  }

  std::lock_guard<std::mutex> const _lock_job_map_mutex (m_job_map_mutex);

  auto job_it (m_jobs.find(*e->job_id()));
  if (job_it != m_jobs.end())
  {
    send_event_to_parent<sdpa::events::SubmitJobAckEvent> (*e->job_id());
    return;
  }

  std::shared_ptr<DRTSImpl::Job> job
    ( std::make_shared<DRTSImpl::Job> ( *e->job_id()
                                      , e->activity()
                                      , e->implementation()
                                      , e->workers()
                                      )
    );

  m_pending_jobs.put (job);

  send_event_to_parent<sdpa::events::SubmitJobAckEvent> (job->id);
  m_jobs.emplace (job->id, job);
}

void DRTSImpl::handleCancelJobEvent
  (fhg::com::p2p::address_t const&, const sdpa::events::CancelJobEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  auto job_it (m_jobs.find (e->job_id()));

  _log_emitter.emit ( "got cancelation request for job: " + e->job_id()
                    , fhg::logging::legacy::category_level_trace
                    );

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("cancel_job for unknown job");
  }

  Job::state_t job_state (Job::PENDING);
  if (job_it->second->state.compare_exchange_strong (job_state, Job::CANCELED))
  {
    _log_emitter.emit ( "canceling pending job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
    send_event_to_parent<sdpa::events::CancelJobAckEvent> (job_it->second->id);
  }
  else if (job_state == DRTSImpl::Job::RUNNING)
  {
    _log_emitter.emit ( "trying to cancel running job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
    std::lock_guard<std::mutex> const _lock_currently_executed_tasks
      (_currently_executed_tasks_mutex);
    auto task_it
      (_currently_executed_tasks.find (e->job_id()));
    if (task_it != _currently_executed_tasks.end())
    {
      task_it->second->state = wfe_task_t::CANCELED;
      task_it->second->context.module_call_do_cancel();
    }
  }
  else if (job_state == DRTSImpl::Job::FAILED)
  {
    _log_emitter.emit ( "cancel_job for failed job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
  }
  else if (job_state == DRTSImpl::Job::CANCELED)
  {
    _log_emitter.emit ( "cancel_job for canceled job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
  }
  else // if (job_state == DRTSImpl::Job::FINISHED)
  {
    _log_emitter.emit ( "cancel_job for finished job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
  }
}

void DRTSImpl::handleJobFailedAckEvent
  (fhg::com::p2p::address_t const&, const sdpa::events::JobFailedAckEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  auto job_it (m_jobs.find (e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_failed_ack for unknown job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent
  (fhg::com::p2p::address_t const&, const sdpa::events::JobFinishedAckEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  auto job_it (m_jobs.find (e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_finished_ack for unknown job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::event_thread()
try
{
  for (;;)
  {
    auto const event (m_event_queue.get());
    try
    {
      event->handleBy (_peer.address(), this);
    }
    catch (std::exception const&)
    {
      send_event_to_parent<sdpa::events::ErrorEvent>
        ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
        , fhg::util::current_exception_printer (": ").string()
        );
    }
  }
}
catch (decltype (m_event_queue)::interrupted const&)
{
}

void DRTSImpl::emit_gantt
  (wfe_task_t const& task, sdpa::daemon::NotificationEvent::state_t state)
{
  _log_emitter.emit_message
    ( { sdpa::daemon::NotificationEvent
          ({m_my_name}, task.id, state, task.activity).encoded()
      , sdpa::daemon::gantt_log_category
      }
    );
}

void DRTSImpl::job_execution_thread()
try
{
  bool worker_was_tainted (false);

  for (;;)
  {
    if (worker_was_tainted)
    {
      _log_emitter.emit
        ( "Worker process was tainted from previous job. "
          "Aborting to avoid corrupt job execution."
        , fhg::logging::legacy::category_level_error
        );

      std::abort();
    }

    std::shared_ptr<DRTSImpl::Job> job (m_pending_jobs.get());

    Job::state_t expeceted_job_state (Job::PENDING);
    if (!job->state.compare_exchange_strong (expeceted_job_state, Job::RUNNING))
    {
      //! \note Can only be CANCELED, thus already sent a CancelJobAck
      //! in handleCancelJob, and can just be removed.
      std::lock_guard<std::mutex> const _ (m_job_map_mutex);
      m_jobs.erase (job->id);

      continue;
    }

    try
    {
      job->result = we::type::Activity();

      wfe_task_t task
        (job->id, job->activity, job->target_impl, m_my_name, job->workers, _log_emitter);

      emit_gantt (task, sdpa::daemon::NotificationEvent::STATE_STARTED);

      auto const generic_on_failure
        ( [&]
          {
            task.state = wfe_task_t::FAILED;
            job->message = fhg::util::current_exception_printer (": ").string();
          }
        );

      try
      {
        //! \todo there is a race between putting it into
        //! _currently_executed_tasks and actually starting it, as
        //! well as between finishing execution and removing: a cancel
        //! between the two means to call on_cancel() on a module call
        //! not yet or no longer executed.
        {
          std::lock_guard<std::mutex> const _ (_currently_executed_tasks_mutex);
          _currently_executed_tasks.emplace (job->id, &task);
        }

        task.activity.execute
          ( m_loader
          , _virtual_memory_api
          , _shared_memory
          , task.target_impl
          , &task.context
          );
      }
      catch (drts::worker::context::cancelled const&)
      {
        task.state = wfe_task_t::CANCELED;
      }
      catch (we::loader::module_does_not_unload const&)
      {
        worker_was_tainted = true;
        generic_on_failure();
      }
      catch (we::loader::function_does_not_unload const&)
      {
        worker_was_tainted = true;
        generic_on_failure();
      }
      catch (...)
      {
        generic_on_failure();
      }

      {
        std::lock_guard<std::mutex> const _ (_currently_executed_tasks_mutex);
        _currently_executed_tasks.erase (job->id);
      }

      //! \note failing or canceling overwrites
      if (task.state == wfe_task_t::PENDING)
      {
        task.state = wfe_task_t::FINISHED;
        job->result = std::move (task.activity);
      }

      if (wfe_task_t::FINISHED == task.state)
      {
        _log_emitter.emit ( "task finished: " + task.id
                          , fhg::logging::legacy::category_level_trace
                          );
      }
      else if (wfe_task_t::FAILED == task.state)
      {
        _log_emitter.emit ( "task failed: " + task.id + ": " + job->message
                          , fhg::logging::legacy::category_level_error
                          );
      }
      else if (wfe_task_t::CANCELED == task.state)
      {
        _log_emitter.emit ( "task canceled: " + task.id
                          , fhg::logging::legacy::category_level_info
                          );
      }

      emit_gantt ( task
                 , task.state == wfe_task_t::FINISHED ? sdpa::daemon::NotificationEvent::STATE_FINISHED
                 : task.state == wfe_task_t::CANCELED ? sdpa::daemon::NotificationEvent::STATE_CANCELED
                 : task.state == wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN ? sdpa::daemon::NotificationEvent::STATE_FAILED
                 : task.state == wfe_task_t::FAILED ? sdpa::daemon::NotificationEvent::STATE_FAILED
                 : throw std::logic_error {"invalid enum value"}
                 );
      job->state = task.state == wfe_task_t::FINISHED ? DRTSImpl::Job::FINISHED
                 : task.state == wfe_task_t::CANCELED ? DRTSImpl::Job::CANCELED
                 : task.state == wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN ? DRTSImpl::Job::CANCELED_DUE_TO_WORKER_SHUTDOWN
                 : task.state == wfe_task_t::FAILED ? DRTSImpl::Job::FAILED
                 : throw std::logic_error {"invalid enum value"}
                 ;

    }
    catch (...)
    {
      std::string const error
        ( "unexpected exception during job execution: "
        + fhg::util::current_exception_printer (": ").string()
        );
      _log_emitter.emit (error, fhg::logging::legacy::category_level_error);
      job->state = DRTSImpl::Job::FAILED;

      job->result = std::move (job->activity);
      job->message = error;
    }

    switch (job->state.load())
    {
    case DRTSImpl::Job::FINISHED:
      send_event_to_parent<sdpa::events::JobFinishedEvent>
        (job->id, job->result);

      break;
    case DRTSImpl::Job::FAILED:
      send_event_to_parent<sdpa::events::JobFailedEvent>
        (job->id, job->message);

      break;
    case DRTSImpl::Job::CANCELED:
      send_event_to_parent<sdpa::events::CancelJobAckEvent> (job->id);

      break;

    case DRTSImpl::Job::CANCELED_DUE_TO_WORKER_SHUTDOWN:
      //! \note Do _not_ send anything: will be rescheduled by worker
      //! failure-handling.

      break;

    default:
      throw std::logic_error {"invalid enum value"};
    }
  }
}
catch (decltype (m_pending_jobs)::interrupted const&)
{
}

void DRTSImpl::start_receiver()
{
  _peer.async_recv
    ( [this] (auto received)
      {
        static sdpa::events::Codec codec;

        if (!received.ec())
        {
          if (received.source() != _peer.other_end())
          {
            throw std::runtime_error ("Message from unknown source.");
          }

          m_event_queue.put
            ( sdpa::events::SDPAEvent::Ptr
                ( codec.decode
                    ( std::string ( received.message().data.begin()
                                  , received.message().data.end()
                                  )
                    )
                )
            );

          start_receiver();
        }
        else if (!m_shutting_down)
        {
          if (!_registered)
          {
            _registration_response
              .set_exception
                ( std::make_exception_ptr
                    ( std::system_error
                        (std::make_error_code (std::errc::connection_aborted))
                    )
                );
          }
          else
          {
            _request_stop();
          }
        }
        else
        {
          _log_emitter.emit ( m_my_name + " is shutting down"
                            , fhg::logging::legacy::category_level_trace
                            );
        }
      }
    );
}

template<typename Event, typename... Args>
  void DRTSImpl::send_event_to_parent (Args&&... args)
{
  static sdpa::events::Codec codec;
  _peer.send (codec.encode<Event> (std::forward<Args> (args)...));
}
