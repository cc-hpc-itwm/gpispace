#include <drts/worker/drts.hpp>

//! \todo remove when redoing ctor
#include <fhg/util/getenv.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/hostname.hpp>

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

#include <we/loader/module_call.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.hpp>
#include <we/type/net.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <functional>
#include <random>

//! \note Temporary, while config_variables are passed in as map<>.
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

template<typename T> boost::optional<T> get
  (std::string key, std::map<std::string, std::string> const& vals)
{
  const std::map<std::string, std::string>::const_iterator it (vals.find (key));
  if (it != vals.end())
  {
    return boost::lexical_cast<T> (it->second);
  }
  return boost::none;
}

numa_socket_setter::numa_socket_setter (size_t target_socket)
{
  hwloc_topology_init (&m_topology);
  hwloc_topology_load (m_topology);

  const int depth (hwloc_get_type_depth (m_topology, HWLOC_OBJ_SOCKET));
  if (depth == HWLOC_TYPE_DEPTH_UNKNOWN)
  {
    throw std::runtime_error ("could not get number of sockets");
  }

  const size_t available_sockets (hwloc_get_nbobjs_by_depth (m_topology, depth));

  if (target_socket >= available_sockets)
  {
    throw std::runtime_error
      ( boost::str ( boost::format ("socket out of range: %1%/%2%")
                   % target_socket
                   % (available_sockets-1)
                   )
      );
  }

  const hwloc_obj_t obj
    (hwloc_get_obj_by_type (m_topology, HWLOC_OBJ_SOCKET, target_socket));

  char cpuset_string [256];
  hwloc_bitmap_snprintf (cpuset_string, sizeof(cpuset_string), obj->cpuset);

  if (hwloc_set_cpubind (m_topology, obj->cpuset, HWLOC_CPUBIND_PROCESS) < 0)
  {
    throw std::runtime_error
      ( boost::str
        ( boost::format ("could not bind to socket #%1% with cpuset %2%: %3%")
        % target_socket
        % cpuset_string
        % strerror (errno)
        )
      );
  }
}

numa_socket_setter::~numa_socket_setter()
{
  hwloc_topology_destroy (m_topology);
}

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
              = boost::get<we::type::net_type&> (act.transition().data())
              . fire_expressions_and_extract_activity_random (_engine)
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

    virtual void handle_internally (we::type::activity_t& act, mod_t const& mod) override
    {
      try
      {
        we::loader::module_call ( loader
                                , _virtual_memory_api
                                , _shared_memory
                                , &task.context
                                , act
                                , mod
                                );
      }
      catch (std::exception const& ex)
      {
        throw std::runtime_error
          ( "call to '" + mod.module() + "::" + mod.function() + "'"
          + " failed: " + ex.what()
          );
      }
    }

    virtual void handle_internally (we::type::activity_t&, expr_t const&) override
    {
    }

    virtual void handle_externally (we::type::activity_t& act, net_t const& n) override
    {
      handle_internally (act, n);
    }

    virtual void handle_externally (we::type::activity_t& act, mod_t const& module_call) override
    {
      handle_internally (act, module_call);
    }

    virtual void handle_externally (we::type::activity_t& act, expr_t const& e) override
    {
      handle_internally (act, e);
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
  std::unique_lock<std::mutex> const _ (_currently_executed_tasks_mutex);
  while (!_currently_executed_tasks.empty())
  {
    wfe_task_t *task = _currently_executed_tasks.begin()->second;
    task->state = wfe_task_t::CANCELED;

    _currently_executed_tasks.erase (task->id);
  }
}

namespace
{
  std::set<sdpa::Capability> make_capabilities
    (std::list<std::string> capabilities, std::string worker_name)
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
    , boost::asio::io_service& peer_io_service
    , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
    , std::map<std::string, std::string> config_variables
    , std::string const& kernel_name
    , gpi::pc::client::api_t /*const*/* virtual_memory_api
    , gspc::scoped_allocation /*const*/* shared_memory
    , std::vector<master_info> const& masters
    , std::list<std::string> const& capability_names
    , boost::optional<std::size_t> const& socket
    , std::list<boost::filesystem::path> const& library_path
    , std::size_t backlog_length
    )
  : _logger (fhg::log::Logger::get (kernel_name))
  , _request_stop (request_stop)
  , m_shutting_down (false)
  , m_my_name (kernel_name)
  , _numa_socket_setter ( socket
                        ? numa_socket_setter (*socket)
                        : boost::optional<numa_socket_setter>()
                        )
  , _currently_executed_tasks()
  , m_loader (library_path)
  , _notification_service ( gui_info
                          ? sdpa::daemon::NotificationService
                            (gui_info->first, gui_info->second)
                          : boost::optional<sdpa::daemon::NotificationService>()
                          )
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
  , m_peer ( std::make_shared<fhg::com::peer_t>
               (peer_io_service, fhg::com::host_t ("*"), fhg::com::port_t ("0"))
           )
  , m_peer_thread ( std::make_shared<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
                      (&fhg::com::peer_t::run, m_peer.get())
                  )
  , m_pending_jobs (backlog_length)
  , m_event_thread (&DRTSImpl::event_thread, this)
  , m_execution_thread (&DRTSImpl::job_execution_thread, this)
{
  m_peer->start();

  start_receiver();

  std::set<sdpa::Capability> const capabilities
    (make_capabilities (capability_names, m_my_name));

  for (master_info const& master : masters)
  {
    send_event<sdpa::events::WorkerRegistrationEvent>
      ( m_masters.emplace
          ( std::get<0> (master)
          , m_peer->connect_to (std::get<1> (master), std::get<2> (master))
          ).first->second
      , m_my_name
      , m_pending_jobs.capacity()
      , capabilities
      , false
      , fhg::util::hostname()
      );
  }
}

DRTSImpl::peer_stopper::~peer_stopper()
{
  m_peer->stop();
  m_peer_thread.reset();
  m_peer.reset();
}

DRTSImpl::~DRTSImpl()
{
  m_shutting_down = true;
}

void DRTSImpl::handleWorkerRegistrationAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::WorkerRegistrationAckEvent*)
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
    throw std::runtime_error ("worker_registration_ack for unknown master");
  }

  resend_outstanding_events (master_it);
}

void DRTSImpl::handleSubmitJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent *e)
{
  map_of_masters_t::const_iterator master
    ( std::find_if ( m_masters.cbegin(), m_masters.cend()
                   , [&source] (map_of_masters_t::value_type const& master)
                     {
                       return master.second == source;
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
                                      , e->worker_list()
                                      )
    );

  std::unique_lock<std::mutex> job_map_lock(m_job_map_mutex);

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
        : throw std::runtime_error ("invalid job state")
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
        (event.first, sdpa::events::ErrorEvent::SDPA_EUNKNOWN, ex.what());
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

      wfe_task_t task (job->id, job->description, m_my_name, job->worker_list);

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
        catch (std::exception const& ex)
        {
          task.state = wfe_task_t::FAILED;
          job->message = std::string ("Module call failed: ") + ex.what();
        }
        catch (...)
        {
          task.state = wfe_task_t::FAILED;
          job->message = "UNKNOWN REASON, exception not derived from std::exception";
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
      else if (wfe_task_t::CANCELED == task.state)
      {
      }
      else // if (wfe_task_t::FAILED == task.state)
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
              : task.state == wfe_task_t::FAILED ? NotificationEvent::STATE_FAILED
              : throw std::runtime_error ("bad enum value: task.state")
              , task.activity
              )
            );
        }

      job->state = task.state == wfe_task_t::FINISHED ? DRTSImpl::Job::FINISHED
                 : task.state == wfe_task_t::CANCELED ? DRTSImpl::Job::CANCELED
                 : task.state == wfe_task_t::FAILED ? DRTSImpl::Job::FAILED
                 : throw std::runtime_error ("bad task state");

    }
    catch (std::exception const& ex)
    {
      LLOG ( ERROR, _logger
           , "unexpected exception during job execution: " << ex.what()
           );
      job->state = DRTSImpl::Job::FAILED;

      job->result = job->description;
      job->message = ex.what();
    }

    send_job_result_to_master (job);
  }
}

void DRTSImpl::resend_outstanding_events
  (map_of_masters_t::const_iterator const& master)
{
  LLOG (TRACE, _logger, "resending outstanding notifications to " << master->first);

  std::unique_lock<std::mutex> const _ (m_job_map_mutex);

  for ( std::shared_ptr<DRTSImpl::Job> const& job
      : m_jobs
      | boost::adaptors::map_values
      | boost::adaptors::filtered
          ( [&master] (std::shared_ptr<DRTSImpl::Job> const& j)
            {
              return j->owner == master && j->state >= DRTSImpl::Job::FINISHED;
            }
          )
      )
  {
    LLOG (TRACE, _logger, "resending outcome of job " << job->id);
    send_job_result_to_master (job);
  }
}

void DRTSImpl::send_job_result_to_master (std::shared_ptr<DRTSImpl::Job> const& job)
{
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

  default:
    INVALID_ENUM_VALUE (DRTSImpl::Job::state_t, job->state);
  }
}

void DRTSImpl::start_receiver()
{
  m_peer->async_recv
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
          _request_stop();
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
  Event const event (std::forward<Args> (args)...);
  m_peer->send (destination, codec.encode (&event));
}
