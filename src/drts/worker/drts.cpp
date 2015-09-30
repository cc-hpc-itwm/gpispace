#include <drts/worker/drts.hpp>

#include <util-generic/hostname.hpp>

#include <fhg/util/macros.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>

#include <we/context.hpp>
#include <we/loader/module_call.hpp>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

namespace
{
  struct wfe_exec_context : public we::context
  {
    std::mt19937 _engine;

    wfe_exec_context
      ( we::loader::loader& module_loader
      , gpi::pc::client::api_t /*const*/* virtual_memory_api
      , gspc::scoped_allocation /*const*/* shared_memory
      , wfe_task_t& target
      )
      : loader (module_loader)
      , _virtual_memory_api (virtual_memory_api)
      , _shared_memory (shared_memory)
      , task (target)
    {}

    virtual void handle_internally (we::type::activity_t& act, net_t const&) override
    {
      if (act.transition().net())
      {
        while ( boost::optional<we::type::activity_t> sub
              = boost::get<we::type::net_type> (act.transition().data())
              . fire_expressions_and_extract_activity_random
                  ( _engine
                  , [] (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
                    {
                      throw std::logic_error ("got workflow_response: unsupported in worker");
                    }
                  )
              )
        {
          sub->execute (this);
          act.inject (*sub);

          if (task.state == wfe_task_t::CANCELED)
          {
            break;
          }
        }
      }
    }

    virtual void handle_externally (we::type::activity_t& act, net_t const& n) override
    {
      handle_internally (act, n);
    }

    virtual void handle_externally (we::type::activity_t& act, mod_t const& mod) override
    {
      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            we::loader::module_call ( loader
                                    , _virtual_memory_api
                                    , _shared_memory
                                    , &task.context
                                    , act
                                    , mod
                                    );
          }
        , "call to '" + mod.module() + "::" + mod.function() + "' failed"
        );
    }

  private:
    we::loader::loader& loader;
    gpi::pc::client::api_t /*const*/* _virtual_memory_api;
    gspc::scoped_allocation /*const*/* _shared_memory;
    wfe_task_t& task;
  };
}

DRTSImpl::mark_remaining_tasks_as_canceled_helper::~mark_remaining_tasks_as_canceled_helper()
{
  std::unique_lock<std::mutex> const currently_executed_tasks_lock
    (_currently_executed_tasks_mutex);

  for (auto& task : _currently_executed_tasks | boost::adaptors::map_values)
  {
    task->state = wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN;
    task->context.module_call_do_cancel();
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
    , std::unique_ptr<sdpa::daemon::NotificationService> gui_notification_service
    , std::string const& kernel_name
    , gpi::pc::client::api_t /*const*/* virtual_memory_api
    , gspc::scoped_allocation /*const*/* shared_memory
    , std::vector<master_info> const& masters
    , std::vector<std::string> const& capability_names
    , std::vector<boost::filesystem::path> const& library_path
    , std::size_t backlog_length
    , fhg::log::Logger& logger
    )
  : _logger (logger)
  , _request_stop (request_stop)
  , m_shutting_down (false)
  , m_my_name (kernel_name)
  , _currently_executed_tasks()
  , m_loader ({library_path.begin(), library_path.end()})
  , _notification_service (std::move (gui_notification_service))
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
  , m_pending_jobs (backlog_length)
  , _peer ( std::move (peer_io_service)
          , fhg::com::host_t ("*"), fhg::com::port_t ("0")
          )
  , m_event_thread (&DRTSImpl::event_thread, this)
  , m_execution_thread (&DRTSImpl::job_execution_thread, this)
{
  start_receiver();

  std::set<sdpa::Capability> const capabilities
    (make_capabilities (capability_names, m_my_name));

  std::vector<std::future<void>> registration_futures;

  for (master_info const& master : masters)
  {
    fhg::com::p2p::address_t const master_address
      ( m_masters.emplace
          ( std::get<0> (master)
          , _peer.connect_to (std::get<1> (master), std::get<2> (master))
          ).first->second
      );

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
}

void DRTSImpl::handle_worker_registration_response
  ( fhg::com::p2p::address_t const& source
  , sdpa::events::worker_registration_response const* response
  )
{
  map_of_masters_t::const_iterator master_it
    ( std::find_if ( m_masters.cbegin(), m_masters.cend()
                   , [&source] (map_of_masters_t::value_type const& master)
                     {
                       return master.second == source;
                     }
                   )
    );

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
  map_of_masters_t::const_iterator master
    ( std::find_if ( m_masters.cbegin(), m_masters.cend()
                   , [&source] (map_of_masters_t::value_type const& master_)
                     {
                       return master_.second == source;
                     }
                   )
    );

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
    //! being marked as free again at any point, but instead even
    //! remove them all to avoid a legit backlogfull triggering
    //! another job
    std::unique_lock<std::mutex> const _
      (_guard_backlogfull_notified_masters);
    _masters_backlogfull_notified.clear();

    return;
  }

  std::unique_lock<std::mutex> job_map_lock(m_job_map_mutex);

  map_of_jobs_t::iterator job_it (m_jobs.find(*e->job_id()));
  if (job_it != m_jobs.end())
  {
    send_event<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());
    return;
  }

  std::shared_ptr<DRTSImpl::Job> job
    ( std::make_shared<DRTSImpl::Job> ( *e->job_id()
                                      , e->description()
                                      , master
                                      , e->workers()
                                      )
    );

  if (!m_pending_jobs.try_put (job))
  {
    LLOG ( WARN, _logger
         , "cannot accept new job (" << job->id << "), backlog is full."
         );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EBACKLOGFULL
      , "I am busy right now, please try again later!"
      , *e->job_id()
      );

    std::unique_lock<std::mutex> _ (_guard_backlogfull_notified_masters);
    _masters_backlogfull_notified.emplace (source);

    return;
  }

  send_event<sdpa::events::SubmitJobAckEvent> (master->second, job->id);
  m_jobs.emplace (job->id, job);
}

void DRTSImpl::handleCancelJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent *e)
{
  std::unique_lock<std::mutex> job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  LLOG (TRACE, _logger, "got cancelation request for job: " << e->job_id());

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("cancel_job for unknown job");
  }
  if (job_it->second->owner->second != source)
  {
    throw std::runtime_error ("cancel_job for non-owned job");
  }

  Job::state_t job_state (Job::PENDING);
  if (job_it->second->state.compare_exchange_strong (job_state, Job::CANCELED))
  {
    LLOG (TRACE, _logger, "canceling pending job: " << e->job_id());
    send_event<sdpa::events::CancelJobAckEvent>
      (job_it->second->owner->second, job_it->second->id);
  }
  else if (job_state == DRTSImpl::Job::RUNNING)
  {
    LLOG (TRACE, _logger, "trying to cancel running job " << e->job_id());
    std::unique_lock<std::mutex> const _ (_currently_executed_tasks_mutex);
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
    LLOG (TRACE, _logger, "cancel_job for failed job");
  }
  else if (job_state == DRTSImpl::Job::CANCELED)
  {
    LLOG (TRACE, _logger, "cancel_job for canceled job");
  }
  else // if (job_state == DRTSImpl::Job::FINISHED)
  {
    LLOG (TRACE, _logger, "cancel_job for finished job");
  }
}

void DRTSImpl::handleJobFailedAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent *e)
{
  std::unique_lock<std::mutex> job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_failed_ack for unknown job");
  }
  if (job_it->second->owner->second != source)
  {
    throw std::runtime_error ("job_failed_ack for non-owned job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent *e)
{
  std::unique_lock<std::mutex> job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  if (job_it == m_jobs.end())
  {
    throw std::runtime_error ("job_finished_ack for unknown job");
  }
  if (job_it->second->owner->second != source)
  {
    throw std::runtime_error ("job_finished_ack for non-owned job");
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleDiscoverJobStatesEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent* event)
{
  std::unique_lock<std::mutex> const _ (m_job_map_mutex);

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

void DRTSImpl::job_execution_thread()
{
  for (;;)
  {
    std::shared_ptr<DRTSImpl::Job> job;
    bool notify_can_take_jobs;
    std::tie (job, notify_can_take_jobs) = m_pending_jobs.get();

    if (notify_can_take_jobs)
    {
      std::unique_lock<std::mutex> const _ (_guard_backlogfull_notified_masters);
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
      std::unique_lock<std::mutex> job_map_lock (m_job_map_mutex);
      m_jobs.erase (job->id);

      continue;
    }

    try
    {
      job->result = we::type::activity_t().to_string();

      wfe_task_t task
        (job->id, job->description, m_my_name, job->workers, _logger);

      {
        std::unique_lock<std::mutex> const _ (_currently_executed_tasks_mutex);
        _currently_executed_tasks.emplace (job->id, &task);
      }

      if (_notification_service)
      {
        using sdpa::daemon::NotificationEvent;
        _notification_service->notify
          ( NotificationEvent
              ( {m_my_name}
              , task.id
              , NotificationEvent::STATE_STARTED
              , task.activity
              )
          );
      }

      if (task.state == wfe_task_t::PENDING)
      {
        try
        {
          wfe_exec_context ctxt
            (m_loader, _virtual_memory_api, _shared_memory, task);

          task.activity.execute (&ctxt);

          //! \note failing or canceling overwrites
          if (task.state == wfe_task_t::PENDING)
          {
            task.state = wfe_task_t::FINISHED;
            job->result = task.activity.to_string();
          }
        }
        catch (...)
        {
          task.state = wfe_task_t::FAILED;
          job->message = "Module call failed: "
            + fhg::util::current_exception_printer (": ").string();
        }
      }

      {
        std::unique_lock<std::mutex> const _ (_currently_executed_tasks_mutex);
        _currently_executed_tasks.erase (job->id);
      }

      if (wfe_task_t::FINISHED == task.state)
      {
        LLOG (TRACE, _logger, "task finished: " << task.id);
      }
      else if (wfe_task_t::FAILED == task.state)
      {
        LLOG (ERROR, _logger, "task failed: " << task.id << ": " << job->message);
      }

      if (_notification_service)
      {
        using sdpa::daemon::NotificationEvent;
        _notification_service->notify
          ( NotificationEvent
              ( {m_my_name}
              , task.id
              , task.state == wfe_task_t::FINISHED ? NotificationEvent::STATE_FINISHED
              : task.state == wfe_task_t::CANCELED ? NotificationEvent::STATE_CANCELED
              : task.state == wfe_task_t::CANCELED_DUE_TO_WORKER_SHUTDOWN ? NotificationEvent::STATE_FAILED
              : task.state == wfe_task_t::FAILED ? NotificationEvent::STATE_FAILED
              : INVALID_ENUM_VALUE (wfe_task_t::state_t, task.state)
              , task.activity
              )
            );
        }

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
      LLOG (ERROR, _logger, error);
      job->state = DRTSImpl::Job::FAILED;

      job->result = job->description;
      job->message = error;
    }

    switch (job->state.load())
    {
    case DRTSImpl::Job::FINISHED:
      send_event<sdpa::events::JobFinishedEvent>
        (job->owner->second, job->id, job->result);

      break;
    case DRTSImpl::Job::FAILED:
      send_event<sdpa::events::JobFailedEvent>
        (job->owner->second, job->id, job->message);

      break;
    case DRTSImpl::Job::CANCELED:
      send_event<sdpa::events::CancelJobAckEvent>
        (job->owner->second, job->id);

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
          LLOG (TRACE, _logger, m_my_name << " is shutting down");
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
