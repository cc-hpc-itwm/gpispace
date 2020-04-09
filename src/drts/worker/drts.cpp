#include <drts/worker/drts.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/events/BacklogNoLongerFullEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
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
#include <we/type/activity.hpp>

#include <fhg/util/macros.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <cstdlib>
#include <random>

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
  state_t state;
  we::type::activity_t activity;
  boost::optional<std::string> const target_impl;
  drts::worker::context context;

  wfe_task_t ( std::string id_
             , we::type::activity_t const& activity
             , boost::optional<std::string> const& target_impl_
             , std::string worker_name
             , std::set<std::string> workers
             , fhg::logging::stream_emitter& logger
             )
    : id (id_)
    , state (PENDING)
    , activity (activity)
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

  for (auto& task : _currently_executed_tasks | boost::adaptors::map_values)
  {
    task->state = wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN;
    task->context.module_call_do_cancel();
  }

  for (auto& job : _jobs | boost::adaptors::map_values)
  {
    job->state = Job::state_t::CANCELED_DUE_TO_WORKER_SHUTDOWN;
  }
}

namespace
{
  std::set<sdpa::Capability> make_capabilities
    (std::vector<std::string> const& capabilities, std::string worker_name)
  {
    std::set<sdpa::Capability> result;
    for (std::string const& cap : capabilities)
    {
      result.emplace (cap, worker_name);
    }
    return result;
  }
}

DRTSImpl::DRTSImpl
    ( std::function<void()> request_stop
    , std::unique_ptr<boost::asio::io_service> peer_io_service
    , std::string const& kernel_name
    , unsigned short comm_port
    , gpi::pc::client::api_t /*const*/* virtual_memory_api
    , gspc::scoped_allocation /*const*/* shared_memory
    , std::vector<master_info> const& masters
    , std::vector<std::string> const& capability_names
    , std::vector<boost::filesystem::path> const& library_path
    , std::size_t backlog_length
    , fhg::logging::stream_emitter& log_emitter
    , fhg::com::Certificates const& certificates
    )
  : _request_stop (request_stop)
  , m_shutting_down (false)
  , m_my_name (kernel_name)
  , _currently_executed_tasks()
  , m_loader ({library_path.begin(), library_path.end()})
  , _log_emitter (log_emitter)
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
  , m_pending_jobs (backlog_length)
  , _peer ( std::move (peer_io_service)
          , fhg::com::host_t ("*")
          , fhg::com::port_t (std::to_string (comm_port))
          , certificates
          )
  , m_event_thread (&DRTSImpl::event_thread, this)
  , _interrupt_event_thread (m_event_queue)
  , m_execution_thread (&DRTSImpl::job_execution_thread, this)
  , _interrupt_execution_thread (m_pending_jobs)
{
  start_receiver();

  std::set<sdpa::Capability> const capabilities
    (make_capabilities (capability_names, m_my_name));

  std::vector<std::future<void>> registration_futures;

  for (master_info const& master : masters)
  {
    fhg::com::p2p::address_t const master_address
      (_peer.connect_to (std::get<0> (master), std::get<1> (master)));
    m_masters.emplace_back (master_address);

    registration_futures.emplace_back
      ( _registration_responses.emplace
          (master_address, std::promise<void>()).first->second.get_future()
      );

    send_event<sdpa::events::WorkerRegistrationEvent>
      ( master_address
      , m_my_name
      , capabilities
      , (_shared_memory != nullptr) ? _shared_memory->size() : 0
      , false
      , fhg::util::hostname()
      );
  }

  fhg::util::wait_and_collect_exceptions (registration_futures);

  _registration_responses.clear();
}

DRTSImpl::~DRTSImpl()
{
  m_shutting_down = true;

  //! \note remove them all to avoid a legit backlogfull triggering
  //! another job: we will reply to all submitjobs with a backlogfull
  //! that should never be followed up with a no-longer-full
  std::lock_guard<std::mutex> const _
    (_guard_backlogfull_notified_masters);
  _masters_backlogfull_notified.clear();
}

void DRTSImpl::handle_worker_registration_response
  ( fhg::com::p2p::address_t const& source
  , sdpa::events::worker_registration_response const* response
  )
{
  auto const master_it
    (std::find (m_masters.cbegin(), m_masters.cend(), source));

  if (master_it == m_masters.cend())
  {
    throw std::runtime_error ("worker_registration_response for unknown master");
  }

  try
  {
    response->get();

    _registration_responses.at (source).set_value();
  }
  catch (...)
  {
    _registration_responses.at (source).set_exception (std::current_exception());
  }
}

void DRTSImpl::handleSubmitJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent *e)
{
  auto const master
    (std::find (m_masters.cbegin(), m_masters.cend(), source));

  if (master == m_masters.cend())
  {
    throw std::runtime_error ("got SubmitJob from unknown source");
  }

  if (!e->job_id())
  {
    throw std::runtime_error ("Received job with an unspecified job id");
  }

  if (m_shutting_down)
  {
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EBACKLOGFULL
      , "abusing backlogfull to stop getting new jobs"
      , *e->job_id()
      );

    //! \note not putting into _masters_backlogfull_notified to avoid
    //! being marked as free again at any point

    return;
  }

  std::lock_guard<std::mutex> const _ (m_job_map_mutex);

  map_of_jobs_t::iterator job_it (m_jobs.find(*e->job_id()));
  if (job_it != m_jobs.end())
  {
    send_event<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());
    return;
  }

  std::shared_ptr<DRTSImpl::Job> job
    ( std::make_shared<DRTSImpl::Job> ( *e->job_id()
                                      , e->activity()
                                      , e->implementation()
                                      , master
                                      , e->workers()
                                      )
    );

  if (!m_pending_jobs.try_put (job))
  {
    _log_emitter.emit ( "cannot accept new job (" + job->id
                      + "), backlog is full."
                      , fhg::logging::legacy::category_level_warn
                      );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EBACKLOGFULL
      , "I am busy right now, please try again later!"
      , *e->job_id()
      );

    std::lock_guard<std::mutex> _ (_guard_backlogfull_notified_masters);
    _masters_backlogfull_notified.emplace (source);

    return;
  }

  send_event<sdpa::events::SubmitJobAckEvent> (*master, job->id);
  m_jobs.emplace (job->id, job);
}

void DRTSImpl::handleCancelJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  _log_emitter.emit ( "got cancelation request for job: " + e->job_id()
                    , fhg::logging::legacy::category_level_trace
                    );

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("cancel_job for unknown job");
  }
  if (*job_it->second->owner != source)
  {
    throw std::runtime_error ("cancel_job for non-owned job");
  }

  Job::state_t job_state (Job::PENDING);
  if (job_it->second->state.compare_exchange_strong (job_state, Job::CANCELED))
  {
    _log_emitter.emit ( "canceling pending job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
    send_event<sdpa::events::CancelJobAckEvent>
      (*job_it->second->owner, job_it->second->id);
  }
  else if (job_state == DRTSImpl::Job::RUNNING)
  {
    _log_emitter.emit ( "trying to cancel running job " + e->job_id()
                      , fhg::logging::legacy::category_level_trace
                      );
    std::lock_guard<std::mutex> const _ (_currently_executed_tasks_mutex);
    std::map<std::string, wfe_task_t *>::iterator task_it
      (_currently_executed_tasks.find(e->job_id()));
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
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_failed_ack for unknown job");
  }
  if (*job_it->second->owner != source)
  {
    throw std::runtime_error ("job_failed_ack for non-owned job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent *e)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_finished_ack for unknown job");
  }
  if (*job_it->second->owner != source)
  {
    throw std::runtime_error ("job_finished_ack for non-owned job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleDiscoverJobStatesEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent* event)
{
  std::lock_guard<std::mutex> const _ (m_job_map_mutex);

  const map_of_jobs_t::iterator job_it (m_jobs.find (event->job_id()));
  send_event<sdpa::events::DiscoverJobStatesReplyEvent>
    ( source
    , event->discover_id()
    , sdpa::discovery_info_t
        ( event->job_id()
        , job_it == m_jobs.end() ? boost::optional<sdpa::status::code>()
        : job_it->second->state == DRTSImpl::Job::PENDING ? sdpa::status::PENDING
        : job_it->second->state == DRTSImpl::Job::RUNNING ? sdpa::status::RUNNING
        : job_it->second->state == DRTSImpl::Job::FINISHED ? sdpa::status::FINISHED
        : job_it->second->state == DRTSImpl::Job::FAILED ? sdpa::status::FAILED
        : job_it->second->state == DRTSImpl::Job::CANCELED ? sdpa::status::CANCELED
        : job_it->second->state == DRTSImpl::Job::CANCELED_DUE_TO_WORKER_SHUTDOWN ? sdpa::status::FAILED
        : INVALID_ENUM_VALUE (DRTSImpl::Job::state_t, job_it->second->state)
        , sdpa::discovery_info_set_t()
        )
    );
}

void DRTSImpl::event_thread()
try
{
  for (;;)
  {
    std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr> event
      (m_event_queue.get());
    try
    {
      event.second->handleBy (event.first, this);
    }
    catch (std::exception const& ex)
    {
      send_event<sdpa::events::ErrorEvent>
        ( event.first
        , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
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

    std::shared_ptr<DRTSImpl::Job> job;
    bool notify_can_take_jobs;
    std::tie (job, notify_can_take_jobs) = m_pending_jobs.get();

    if (notify_can_take_jobs)
    {
      std::lock_guard<std::mutex> const _ (_guard_backlogfull_notified_masters);
      for (const fhg::com::p2p::address_t& master : _masters_backlogfull_notified)
      {
        send_event<sdpa::events::BacklogNoLongerFullEvent> (master);
      }

      _masters_backlogfull_notified.clear();
    }

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
      job->result = we::type::activity_t();

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
                 : INVALID_ENUM_VALUE (wfe_task_t::state_t, task.state)
                 );
      job->state = task.state == wfe_task_t::FINISHED ? DRTSImpl::Job::FINISHED
                 : task.state == wfe_task_t::CANCELED ? DRTSImpl::Job::CANCELED
                 : task.state == wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN ? DRTSImpl::Job::CANCELED_DUE_TO_WORKER_SHUTDOWN
                 : task.state == wfe_task_t::FAILED ? DRTSImpl::Job::FAILED
                 : INVALID_ENUM_VALUE (wfe_task_t::state_t, task.state);

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
      send_event<sdpa::events::JobFinishedEvent>
        (*job->owner, job->id, job->result);

      break;
    case DRTSImpl::Job::FAILED:
      send_event<sdpa::events::JobFailedEvent>
        (*job->owner, job->id, job->message);

      break;
    case DRTSImpl::Job::CANCELED:
      send_event<sdpa::events::CancelJobAckEvent>
        (*job->owner, job->id);

      break;

    case DRTSImpl::Job::CANCELED_DUE_TO_WORKER_SHUTDOWN:
      //! \note Do _not_ send anything: will be rescheduled by worker
      //! failure-handling.

      break;

    default:
      INVALID_ENUM_VALUE (DRTSImpl::Job::state_t, job->state);
    }
  }
}
catch (decltype (m_pending_jobs)::interrupted const&)
{
}

void DRTSImpl::start_receiver()
{
  _peer.async_recv
    ( &m_message
    , [this] ( boost::system::error_code const& ec
             , boost::optional<fhg::com::p2p::address_t> source
             )
      {
        static sdpa::events::Codec codec;

        if (!ec)
        {
          m_event_queue.put
            ( source.get()
            , sdpa::events::SDPAEvent::Ptr
                ( codec.decode
                    (std::string (m_message.data.begin(), m_message.data.end()))
                )
            );

          start_receiver();
        }
        else if (!m_shutting_down)
        {
          if (!_registration_responses.empty())
          {
            _registration_responses.at (source.get())
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
  void DRTSImpl::send_event ( fhg::com::p2p::address_t const& destination
                            , Args&&... args
                            )
{
  static sdpa::events::Codec codec;
  _peer.send (destination, codec.encode<Event> (std::forward<Args> (args)...));
}
