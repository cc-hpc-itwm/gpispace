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
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <functional>

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

wfe_task_t::wfe_task_t (std::string id, std::string worker_name, std::list<std::string> workers)
  : id (id)
  , state (wfe_task_t::PENDING)
  , context (worker_name, workers)
{}

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
    boost::mt19937 _engine;

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
      catch (std::exception const &ex)
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
  boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
  while (! _currently_executed_tasks.empty ())
  {
    wfe_task_t *task = _currently_executed_tasks.begin ()->second;
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
    for (std::string const & cap : capabilities)
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
    )
  : _logger (fhg::log::Logger::get (kernel_name))
  , _request_stop (request_stop)
  , m_shutting_down (false)
  , m_my_name (kernel_name)
  , _numa_socket_setter
      ( get<std::size_t> ("plugin.drts.socket", config_variables)
      ? numa_socket_setter (*get<std::size_t> ("plugin.drts.socket", config_variables))
      : boost::optional<numa_socket_setter>()
      )
  , _currently_executed_tasks()
  , m_loader ( fhg::util::split<std::string, boost::filesystem::path>
               (get<std::string> ("plugin.drts.library_path", config_variables).get_value_or (fhg::util::getenv("PC_LIBRARY_PATH").get_value_or ("")), ':')
             )
  , _notification_service ( gui_info
                          ? sdpa::daemon::NotificationService
                            (gui_info->first, gui_info->second)
                          : boost::optional<sdpa::daemon::NotificationService>()
                          )
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
  , m_pending_jobs (get<std::size_t> ("plugin.drts.backlog", config_variables).get_value_or (3))
  , m_event_thread (&DRTSImpl::event_thread, this)
{
  //! \todo ctor parameters
  const std::list<std::string> master_list
    ( fhg::util::split<std::string, std::string>
      (get<std::string> ("plugin.drts.master", config_variables).get_value_or (""), ',')
    );
  fhg::com::host_t host (get<std::string> ("plugin.drts.host", config_variables).get_value_or ("*"));
  fhg::com::port_t port (get<std::string> ("plugin.drts.port", config_variables).get_value_or ("0"));

  if (master_list.empty())
  {
    throw std::runtime_error ("no masters specified");
  }


  // initialize peer
  m_peer.reset
    (new fhg::com::peer_t (peer_io_service, host, port));
  m_peer_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&fhg::com::peer_t::run, m_peer)
    );
  m_peer->start();

  start_receiver();

  std::set<sdpa::Capability> const capabilities
    ( make_capabilities
        ( fhg::util::split<std::string, std::string>
            ( get<std::string> ("plugin.drts.capabilities", config_variables).get_value_or ("")
            , ','
            )
        , m_my_name
        )
    );

  for (std::string const & master : master_list)
  {
    boost::tokenizer<boost::char_separator<char>> const tok
      (master, boost::char_separator<char> ("%"));

    std::vector<std::string> const parts (tok.begin(), tok.end());

    if (parts.size() != 3)
    {
      throw std::runtime_error
        ("invalid master information: has to be of format 'name%host%port'");
    }

    if (parts[0] == m_my_name)
    {
      throw std::runtime_error ("cannot be my own master!");
    }

    if (m_masters.count (master))
    {
      throw std::runtime_error ("master already specified: " + master);
    }

    send_event<sdpa::events::WorkerRegistrationEvent>
      ( m_masters.emplace
          ( parts[0]
          , m_peer->connect_to
              (fhg::com::host_t (parts[1]), fhg::com::port_t (parts[2]))
          ).first->second
      , m_my_name
      , m_pending_jobs.capacity()
      , capabilities
      , false
      , fhg::util::hostname()
      );
  }

  m_execution_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&DRTSImpl::job_execution_thread, this)
    );
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

  // event handler callbacks
  //    implemented events
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
  if (master_it != m_masters.cend())
  {
    resend_outstanding_events (master_it);
  }
}

void DRTSImpl::handleSubmitJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent *e)
{
  // check master
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

  if (e->job_id())
  {
     map_of_jobs_t::iterator job_it (m_jobs.find(*e->job_id()));
     if (job_it != m_jobs.end())
     {
       send_event<sdpa::events::ErrorEvent>
         ( source
         , sdpa::events::ErrorEvent::SDPA_EJOBEXISTS
         , "The job already exists!"
         , *e->job_id()
         );
      return;
     }
  }
  else
  {
    throw std::runtime_error ("Received job with an unspecified job id");
  }

  boost::shared_ptr<DRTSImpl::Job> job (new DRTSImpl::Job( DRTSImpl::Job::ID(*e->job_id())
                                                 , DRTSImpl::Job::Description(e->description())
                                                 , master
                                                         , e->worker_list()
                                                 )
                                   );

  {
    boost::mutex::scoped_lock job_map_lock(m_job_map_mutex);

    if (!m_pending_jobs.try_put (job))
    {
      LLOG ( WARN, _logger
          , "cannot accept new job (" << job->id() << "), backlog is full."
          );
      send_event<sdpa::events::ErrorEvent>
        ( source
        , sdpa::events::ErrorEvent::SDPA_EBACKLOGFULL
        , "I am busy right now, please try again later!"
        , *e->job_id()
        );

      boost::unique_lock<boost::mutex> _ (_guard_backlogfull_notified_masters);
      _masters_backlogfull_notified.emplace (source);

      return;
    }
    else
    {
      send_event<sdpa::events::SubmitJobAckEvent> (master->second, job->id());
      m_jobs.emplace (job->id(), job);
    }
  }
}

void DRTSImpl::handleCancelJobEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  LLOG (TRACE, _logger, "got cancelation request for job: " << e->job_id());

  if (job_it == m_jobs.end())
  {
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
      , "could not find job " + std::string(e->job_id())
      );
  }
  else if (job_it->second->owner()->second != source)
  {
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EPERM
      , "you are not the owner of job " + std::string(e->job_id())
      );
    return;
  }
  else
  {
    if (  DRTSImpl::Job::PENDING
       == job_it->second->cmp_and_swp_state( DRTSImpl::Job::PENDING
                                           , DRTSImpl::Job::CANCELED
                                           )
       )
    {
      LLOG (TRACE, _logger, "canceling pending job: " << e->job_id());
      send_event<sdpa::events::CancelJobAckEvent>
        (job_it->second->owner()->second, job_it->second->id());
    }
    else if (job_it->second->state() == DRTSImpl::Job::RUNNING)
    {
      LLOG (TRACE, _logger, "trying to cancel running job " << e->job_id());
      boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
      std::map<std::string, wfe_task_t *>::iterator task_it
        (_currently_executed_tasks.find(e->job_id()));
      if (task_it != _currently_executed_tasks.end())
      {
        task_it->second->state = wfe_task_t::CANCELED;
        task_it->second->context.module_call_do_cancel();
      }
    }
    else if (job_it->second->state() == DRTSImpl::Job::FAILED)
    {
      LLOG (TRACE, _logger, "canceling already failed job: " << e->job_id());
    }
    else if (job_it->second->state() == DRTSImpl::Job::CANCELED)
    {
      LLOG (TRACE, _logger, "canceling already canceled job: " << e->job_id());
    }
    else
    {
      LLOG ( WARN, _logger
          , "what shall I do with an already computed job? "
          << "(" << e->job_id() << ")"
          );
    }
  }
}

void DRTSImpl::handleJobFailedAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
  if (job_it == m_jobs.end())
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge failed job: " << e->job_id() << ": not found"
        );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
      , "could not find job " + std::string(e->job_id())
      );
    return;
  }
  else if (job_it->second->owner()->second != source)
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge failed job: " << e->job_id() << ": not owner"
        );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EPERM
      , "you are not the owner of job " + std::string(e->job_id())
      );
    return;
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
  if (job_it == m_jobs.end())
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge finished job: " << e->job_id()
        << ": not found"
        );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
      , "could not find job " + std::string(e->job_id())
      );
    return;
  }
  else if (job_it->second->owner()->second != source)
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge finished job: " << e->job_id()
        << ": not owner"
        );
    send_event<sdpa::events::ErrorEvent>
      ( source
      , sdpa::events::ErrorEvent::SDPA_EPERM
      , "you are not the owner of job " + std::string(e->job_id())
      );
    return;
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleDiscoverJobStatesEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent* event)
{
  boost::mutex::scoped_lock const _ (m_job_map_mutex);

  const map_of_jobs_t::iterator job_it (m_jobs.find (event->job_id()));
  send_event<sdpa::events::DiscoverJobStatesReplyEvent>
    ( source
    , event->discover_id()
    , sdpa::discovery_info_t
        ( event->job_id()
        , job_it == m_jobs.end() ? boost::optional<sdpa::status::code>()
        : job_it->second->state() == DRTSImpl::Job::PENDING ? sdpa::status::PENDING
        : job_it->second->state() == DRTSImpl::Job::RUNNING ? sdpa::status::RUNNING
        : job_it->second->state() == DRTSImpl::Job::FINISHED ? sdpa::status::FINISHED
        : job_it->second->state() == DRTSImpl::Job::FAILED ? sdpa::status::FAILED
        : job_it->second->state() == DRTSImpl::Job::CANCELED ? sdpa::status::CANCELED
        : throw std::runtime_error ("invalid job state")
        , sdpa::discovery_info_set_t()
        )
    );
}

  // threads
void DRTSImpl::event_thread ()
{
  for (;;)
  {
    std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr> event
      (m_event_queue.get());
    event.second->handleBy (event.first, this);
  }
}

void DRTSImpl::job_execution_thread ()
{
  for (;;)
  {
    boost::shared_ptr<DRTSImpl::Job> job;
    bool notify_can_take_jobs;
    std::tie (job, notify_can_take_jobs) = m_pending_jobs.get();

    if (notify_can_take_jobs)
    {
      boost::unique_lock<boost::mutex> _ (_guard_backlogfull_notified_masters);
      for (const fhg::com::p2p::address_t& master : _masters_backlogfull_notified)
      {
        send_event<sdpa::events::BacklogNoLongerFullEvent> (master);
      }

      _masters_backlogfull_notified.clear();
    }

    if (DRTSImpl::Job::PENDING == job->cmp_and_swp_state( DRTSImpl::Job::PENDING
                                                    , DRTSImpl::Job::RUNNING
                                                    )
       )
    {
      try
      {
        job->set_result (we::type::activity_t().to_string());

        {
          wfe_task_t task (job->id(), m_my_name, job->worker_list());

          try
          {
            task.activity = we::type::activity_t (job->description());
          }
          catch (std::exception const & ex)
          {
            throw std::runtime_error
              (std::string ("could not parse activity: ") + ex.what());
          }

            {
              boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
              _currently_executed_tasks.emplace (job->id(), &task);
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
                  job->set_result (task.activity.to_string());
                }
              }
              catch (std::exception const & ex)
              {
                task.state = wfe_task_t::FAILED;
                job->set_message (std::string ("Module call failed: ") + ex.what());
              }
              catch (...)
              {
                task.state = wfe_task_t::FAILED;
                job->set_message ("UNKNOWN REASON, exception not derived from std::exception");
              }
            }

            {
              boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
              _currently_executed_tasks.erase (job->id());
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
              LLOG (ERROR, _logger, "task failed: " << task.id << ": " << job->message());
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

            job->set_state (task.state == wfe_task_t::FINISHED ? DRTSImpl::Job::FINISHED
              : task.state == wfe_task_t::CANCELED ? DRTSImpl::Job::CANCELED
              : task.state == wfe_task_t::FAILED ? DRTSImpl::Job::FAILED
              : throw std::runtime_error ("bad task state"));
        }
      }
      catch (std::exception const & ex)
      {
        LLOG ( ERROR, _logger
            , "unexpected exception during job execution: " << ex.what()
            );
        job->set_state (DRTSImpl::Job::FAILED);

        job->set_result (job->description());
        job->set_message (ex.what());
      }

      send_job_result_to_master (job);
    }
    else
    {
      boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
      map_of_jobs_t::iterator job_it (m_jobs.find(job->id()));
      if (job_it != m_jobs.end())
      {
        m_jobs.erase(job_it);
      }
    }
  }
}

void DRTSImpl::resend_outstanding_events
  (map_of_masters_t::const_iterator const& master)
{
  LLOG (TRACE, _logger, "resending outstanding notifications to " << master->first);

  boost::mutex::scoped_lock const _ (m_job_map_mutex);

  for ( boost::shared_ptr<DRTSImpl::Job> const& job
      : m_jobs
      | boost::adaptors::map_values
      | boost::adaptors::filtered
          ( [&master] (boost::shared_ptr<DRTSImpl::Job> const& j)
            {
              return j->owner() == master && j->state() >= DRTSImpl::Job::FINISHED;
            }
          )
      )
  {
    LLOG (TRACE, _logger, "resending outcome of job " << job->id());
    send_job_result_to_master (job);
  }
}

void DRTSImpl::send_job_result_to_master (boost::shared_ptr<DRTSImpl::Job> const & job)
{
  switch (job->state())
  {
  case DRTSImpl::Job::FINISHED:
    send_event<sdpa::events::JobFinishedEvent>
      (job->owner()->second, job->id(), job->result());
    break;
  case DRTSImpl::Job::FAILED:
    {
      send_event<sdpa::events::JobFailedEvent>
        (job->owner()->second, job->id(), job->message());
    }
    break;
  case DRTSImpl::Job::CANCELED:
    {
      send_event<sdpa::events::CancelJobAckEvent>
        (job->owner()->second, job->id());
    }
    break;
  default:
    throw std::runtime_error ("invalid job state in send_job_result_to_master");
  }
}

void DRTSImpl::start_receiver()
{
  m_peer->async_recv
    ( &m_message
    , [this] ( boost::system::error_code const & ec
             , boost::optional<fhg::com::p2p::address_t> source
             )
      {
        static sdpa::events::Codec codec;

        if (! ec)
        {
          // convert m_message to event
          try
          {
            m_event_queue.put
              ( source.get()
              , sdpa::events::SDPAEvent::Ptr
                  ( codec.decode
                      (std::string (m_message.data.begin(), m_message.data.end()))
                  )
              );
          }
          catch (std::exception const & ex)
          {
            LLOG (WARN, _logger, "could not handle incoming message: " << ex.what());
          }
          start_receiver();
        }
        else if (! m_shutting_down)
        {
          if (m_message.header.src != m_peer->address() && !!source)
          {
            _request_stop();
          }
          else
          {
            LLOG (TRACE, _logger, m_my_name << " is shutting down");
          }
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

DRTSImpl::Job::Job( Job::ID const &jobid
                  , Job::Description const &description
                  , owner_type const& owner
                  , std::list<std::string> const& worker_list
                  )
  : m_id (jobid.value)
  , m_input_description (description.value)
  , m_owner (owner)
  , m_state (Job::PENDING)
  , m_result ()
  , m_message ("")
  , m_worker_list (worker_list)
{}

DRTSImpl::Job::state_t DRTSImpl::Job::cmp_and_swp_state( Job::state_t expected
                                                       , Job::state_t newstate
                                                       )
{
  lock_type lock (m_mutex);
  state_t old_state = m_state;
  if (old_state == expected)
  {
    m_state = newstate;
  }
  return old_state;
}
