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

WFEImpl::WFEImpl
  ( fhg::log::Logger::ptr_t logger
  , boost::optional<std::size_t> target_socket
  , std::string search_path
  , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
  , std::string worker_name
  , gpi::pc::client::api_t /*const*/* virtual_memory_api
  , gspc::scoped_allocation /*const*/* shared_memory
  )
  : _logger (logger)
  , _numa_socket_setter ( target_socket
                        ? numa_socket_setter (*target_socket)
                        : boost::optional<numa_socket_setter>()
                        )
  , _worker_name (worker_name)
  , _currently_executed_tasks()
  , m_loader ( fhg::util::split<std::string, boost::filesystem::path>
               (search_path, ':')
             )
  , _notification_service ( gui_info
                          ? sdpa::daemon::NotificationService
                            (gui_info->first, gui_info->second)
                          : boost::optional<sdpa::daemon::NotificationService>()
                          )
  , _virtual_memory_api (virtual_memory_api)
  , _shared_memory (shared_memory)
{
  {
    // TODO: figure out, why this doesn't work as it is supposed to
    // adjust ld_library_path
    std::string ld_library_path (fhg::util::getenv("LD_LIBRARY_PATH").get_value_or (""));
    ld_library_path = search_path + ":" + ld_library_path;
    setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), true);
  }
}

WFEImpl::~WFEImpl()
{
  {
    boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
    while (! _currently_executed_tasks.empty ())
    {
      wfe_task_t *task = _currently_executed_tasks.begin ()->second;
      task->state = wfe_task_t::CANCELED;
      task->error_message = "plugin shutdown";

      _currently_executed_tasks.erase (task->id);
    }
  }
}

void WFEImpl::emit_task (const wfe_task_t& task)
{
  if (_notification_service)
  {
    using sdpa::daemon::NotificationEvent;
    _notification_service->notify
      ( NotificationEvent
        ( {_worker_name}
        , task.id
        , task.state == wfe_task_t::PENDING ? NotificationEvent::STATE_STARTED
        : task.state == wfe_task_t::FINISHED ? NotificationEvent::STATE_FINISHED
        : task.state == wfe_task_t::CANCELED ? NotificationEvent::STATE_CANCELED
        : task.state == wfe_task_t::FAILED ? NotificationEvent::STATE_FAILED
        : throw std::runtime_error ("bad enum value: task.state")
        , task.activity
        )
      );
  }
}

int WFEImpl::execute ( std::string const &job_id
                     , std::string const &job_description
                     , we::type::activity_t & result
                     , std::string & error_message
                     , std::list<std::string> const & worker_list
                     )
{
  wfe_task_t task (job_id, _worker_name, worker_list);

  try
  {
    task.activity = we::type::activity_t (job_description);
  }
  catch (std::exception const & ex)
  {
    LLOG (ERROR, _logger, "could not parse activity: " << ex.what());
    task.state = wfe_task_t::FAILED;
    error_message = std::string ("Invalid job description: ") + ex.what();

    return DRTSImpl::Job::FAILED;
  }

  {
    boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
    _currently_executed_tasks.emplace (job_id, &task);
  }

  emit_task (task);

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
        result = task.activity;
      }
    }
    catch (std::exception const & ex)
    {
      task.state = wfe_task_t::FAILED;
      task.error_message = std::string ("Module call failed: ") + ex.what();
      error_message = task.error_message;
    }
    catch (...)
    {
      task.state = wfe_task_t::FAILED;
      task.error_message =
        "UNKNOWN REASON, exception not derived from std::exception";
      error_message = task.error_message;
    }
  }

  {
    boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
    _currently_executed_tasks.erase (job_id);
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
    LLOG (ERROR, _logger, "task failed: " << task.id << ": " << task.error_message);
  }

  emit_task (task);

  return task.state == wfe_task_t::FINISHED ? DRTSImpl::Job::FINISHED
    : task.state == wfe_task_t::CANCELED ? DRTSImpl::Job::CANCELED
    : task.state == wfe_task_t::FAILED ? DRTSImpl::Job::FAILED
    : throw std::runtime_error ("bad task state");
}

void WFEImpl::cancel (std::string const &job_id)
{
  boost::mutex::scoped_lock const _ (_currently_executed_tasks_mutex);
  std::map<std::string, wfe_task_t *>::iterator task_it
    (_currently_executed_tasks.find(job_id));
  if (task_it != _currently_executed_tasks.end())
  {
    task_it->second->state = wfe_task_t::CANCELED;
    task_it->second->context.module_call_do_cancel();
  }
}

DRTSImpl::master_network_info::master_network_info
    (std::string const& host, std::string const& port)
  : host (host)
  , port (port)
  , address (boost::none)
{}

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
  , m_wfe ( _logger
          , get<std::size_t> ("plugin.drts.socket", config_variables)
          , get<std::string> ("plugin.drts.library_path", config_variables).get_value_or (fhg::util::getenv("PC_LIBRARY_PATH").get_value_or (""))
          , gui_info
          , m_my_name
          , virtual_memory_api
          , shared_memory
          )
  , m_max_reconnect_attempts (get<std::size_t> ("plugin.drts.max_reconnect_attempts", config_variables).get_value_or (0))
  , m_reconnect_counter (0)
  , m_backlog_size (get<std::size_t> ("plugin.drts.backlog", config_variables).get_value_or (3))
{
  //! \todo ctor parameters
  const std::list<std::string> master_list
    ( fhg::util::split<std::string, std::string>
      (get<std::string> ("plugin.drts.master", config_variables).get_value_or (""), ',')
    );
  const std::list<std::string> capability_list
    ( fhg::util::split<std::string, std::string>
      (get<std::string> ("plugin.drts.capabilities", config_variables).get_value_or (""), ',')
    );
  fhg::com::host_t host (get<std::string> ("plugin.drts.host", config_variables).get_value_or ("*"));
  fhg::com::port_t port (get<std::string> ("plugin.drts.port", config_variables).get_value_or ("0"));

  if (master_list.empty())
  {
    throw std::runtime_error ("no masters specified");
  }

  // parse virtual capabilities
  for (std::string const & cap : capability_list)
  {
    m_virtual_capabilities.emplace (cap, m_my_name);
  }

  m_event_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&DRTSImpl::event_thread, this)
    );

  // initialize peer
  m_peer.reset
    (new fhg::com::peer_t (peer_io_service, host, port));
  m_peer_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&fhg::com::peer_t::run, m_peer)
    );
  m_peer->start();

  start_receiver();

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

    if ( !m_masters.emplace ( parts[0]
                            , master_network_info (parts[1], parts[2])
                            ).second
       )
    {
      LLOG ( WARN, _logger
          , "master already specified, ignoring new one: " << master
          );
    }
  }

  m_execution_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&DRTSImpl::job_execution_thread, this)
    );

  start_connect ();
}

DRTSImpl::~DRTSImpl()
{
  m_shutting_down = true;

  m_execution_thread.reset();
  m_event_thread.reset();

  if (m_peer)
  {
    m_peer->stop();
  }

  m_peer_thread.reset();
  m_peer.reset();
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
                       return master.second.address == source;
                     }
                   )
    );
  if (master_it != m_masters.cend())
  {
    {
      boost::mutex::scoped_lock const _ (m_capabilities_mutex);

      if (!m_virtual_capabilities.empty())
      {
        send_event ( source
                   , new sdpa::events::CapabilitiesGainedEvent
                       (m_virtual_capabilities)
                   );
      }
    }

    resend_outstanding_events (master_it);

    {
      boost::mutex::scoped_lock lock_reconnect_counter (m_reconnect_counter_mutex);
      m_reconnect_counter = 0;
    }
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
                       return master.second.address == source;
                     }
                   )
    );

  if (master == m_masters.cend())
  {
    throw std::runtime_error ("got SubmitJob from unknown source");
  }
  else if (! master->second.address)
  {
    throw std::runtime_error ("got SubmitJob from not yet connected master");
  }

  if (e->job_id())
  {
     map_of_jobs_t::iterator job_it (m_jobs.find(*e->job_id()));
     if (job_it != m_jobs.end())
     {
       send_event ( source
                  , new sdpa::events::ErrorEvent
                      ( sdpa::events::ErrorEvent::SDPA_EJOBEXISTS
                      , "The job already exists!"
                      , *e->job_id()
                      )
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
                                                 )
                                   );

  job->worker_list (e->worker_list ());

  {
    boost::mutex::scoped_lock job_map_lock(m_job_map_mutex);

    if (m_backlog_size && m_pending_jobs.INDICATES_A_RACE_size() >= m_backlog_size)
    {
      LLOG ( WARN, _logger
          , "cannot accept new job (" << job->id() << "), backlog is full."
          );
      send_event ( source
                 , new sdpa::events::ErrorEvent
                   ( sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                   , "I am busy right now, please try again later!"
                   , *e->job_id()
                   ));
      return;
    }
    else
    {
      send_event ( *master->second.address
                 , new sdpa::events::SubmitJobAckEvent (job->id())
                 );
      m_jobs.emplace (job->id(), job);

      m_pending_jobs.put(job);
    }
  }

  //    boost::mutex::scoped_lock lock(m_job_arrived_mutex);
  m_job_arrived.notify_all();
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
    send_event( source
              , new sdpa::events::ErrorEvent
                ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                , "could not find job " + std::string(e->job_id())
                ));
  }
  else if (*job_it->second->owner()->second.address != source)
  {
    send_event ( source
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
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
      send_event
        ( *job_it->second->owner()->second.address
        , new sdpa::events::CancelJobAckEvent (job_it->second->id())
        );
    }
    else if (job_it->second->state() == DRTSImpl::Job::RUNNING)
    {
      LLOG (TRACE, _logger, "trying to cancel running job " << e->job_id());
      m_wfe.cancel (e->job_id());
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
    send_event ( source
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (*job_it->second->owner()->second.address != source)
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge failed job: " << e->job_id() << ": not owner"
        );
    send_event ( source
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
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
    send_event ( source
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (*job_it->second->owner()->second.address != source)
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge finished job: " << e->job_id()
        << ": not owner"
        );
    send_event ( source
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleDiscoverJobStatesEvent
  (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent* event)
{
  boost::mutex::scoped_lock const _ (m_job_map_mutex);

  const map_of_jobs_t::iterator job_it (m_jobs.find (event->job_id()));
  send_event ( source
             , new sdpa::events::DiscoverJobStatesReplyEvent
               ( event->discover_id()
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
    boost::shared_ptr<DRTSImpl::Job> job = m_pending_jobs.get();

    if (DRTSImpl::Job::PENDING == job->cmp_and_swp_state( DRTSImpl::Job::PENDING
                                                    , DRTSImpl::Job::RUNNING
                                                    )
       )
    {
      try
      {
        const boost::posix_time::ptime started
          (boost::posix_time::microsec_clock::universal_time());

        LLOG (TRACE, _logger, "executing job " << job->id());

        we::type::activity_t result;
        std::string error_message;
        int ec = m_wfe.execute ( job->id()
                                , job->description()
                                , result
                                , error_message
                                , job->worker_list ()
                                );
        job->set_result (result.to_string());

        const boost::posix_time::ptime completed
          (boost::posix_time::microsec_clock::universal_time());

        LLOG ( TRACE, _logger
            , "job returned."
            << " error-message := " << job->message()
            << " total-time := " << (completed - started)
            );

        job->set_state (DRTSImpl::Job::state_t (ec));

        if (ec == DRTSImpl::Job::FAILED)
        {
          job->set_message (error_message);
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

      {
        boost::mutex::scoped_lock lock(m_job_computed_mutex);
        m_job_computed.notify_one();
      }
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
    send_event ( *job->owner()->second.address
               , new sdpa::events::JobFinishedEvent (job->id(), job->result())
               );
    break;
  case DRTSImpl::Job::FAILED:
    {
      send_event
        ( *job->owner()->second.address
        , new sdpa::events::JobFailedEvent (job->id(), job->message())
        );
    }
    break;
  case DRTSImpl::Job::CANCELED:
    {
      send_event
        ( *job->owner()->second.address
        , new sdpa::events::CancelJobAckEvent (job->id())
        );
    }
    break;
  default:
    throw std::runtime_error ("invalid job state in send_job_result_to_master");
  }
}

void DRTSImpl::request_registration_soon()
{
  _registration_threads.start
    (std::bind (&DRTSImpl::request_registration_after_sleep, this));
}
void DRTSImpl::request_registration_after_sleep()
{
  boost::this_thread::sleep (boost::posix_time::milliseconds (2500));
  start_connect();
}

void DRTSImpl::start_connect ()
{
  bool at_least_one_disconnected = false;
  bool not_connected_to_anyone = true;

  for ( map_of_masters_t::iterator master_it (m_masters.begin())
      ; master_it != m_masters.end()
      ; ++master_it
      )
  {
    if (! master_it->second.address)
    {
      sdpa::events::WorkerRegistrationEvent::Ptr evt
        (new sdpa::events::WorkerRegistrationEvent ( m_my_name
                                                   , m_backlog_size
                                                   , sdpa::capabilities_set_t()
                                                   , false
                                                   , fhg::util::hostname()
                                                   )
        );

      try
      {
        master_it->second.address = m_peer->connect_to
          (master_it->second.host, master_it->second.port);
        send_event(*master_it->second.address, evt);
      }
      catch (boost::system::system_error const& ex)
      {
        if (  ex.code() == boost::system::errc::no_such_process
           || ex.code() == boost::asio::error::connection_refused
           )
        {
          LLOG ( WARN, _logger
               , "could not connect to master '" << master_it->first << "' := " << ex.what()
               );
        }
        else
        {
          LLOG ( ERROR, _logger
               , "could not connect to master '" << master_it->first << "' := " << ex.what()
               );
          throw;
        }
      }

      at_least_one_disconnected = true;
    }
    else
    {
      not_connected_to_anyone = false;
    }
  }

  if (not_connected_to_anyone)
  {
    if (m_max_reconnect_attempts)
    {
      boost::mutex::scoped_lock lock_reconnect_rounter (m_reconnect_counter_mutex);
      if (m_reconnect_counter < m_max_reconnect_attempts)
      {
        ++m_reconnect_counter;
      }
      else
      {
        LLOG ( WARN, _logger
            , "still not connected after " << m_reconnect_counter
            << " trials: shutting down"
            );
        _request_stop();
        return;
      }
    }
  }


  if (at_least_one_disconnected and not m_shutting_down)
  {
    request_registration_soon();
  }
}

void DRTSImpl::start_receiver()
{
  m_peer->async_recv(&m_message, std::bind( &DRTSImpl::handle_recv
                                          , this
                                          , std::placeholders::_1
                                          , std::placeholders::_2
                                          )
                    );
}

void DRTSImpl::handle_recv ( boost::system::error_code const & ec
                           , boost::optional<fhg::com::p2p::address_t> source
                           )
{
  static sdpa::events::Codec codec;

  if (! ec)
  {
    // convert m_message to event
    try
    {
      dispatch_event
        ( source.get()
        , sdpa::events::SDPAEvent::Ptr
          (codec.decode (std::string ( m_message.data.begin()
                                     , m_message.data.end()
                                     )
                        )
          ));
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
      map_of_masters_t::iterator master
        ( std::find_if ( m_masters.begin(), m_masters.end()
                       , [&source] (map_of_masters_t::value_type const& master)
                         {
                           return master.second.address == source;
                         }
                       )
        );
      if (master != m_masters.end())
      {
        master->second.address = boost::none;

        request_registration_soon();
      }

      start_receiver();
    }
    else
    {
      LLOG (TRACE, _logger, m_my_name << " is shutting down");
    }
  }
}

void DRTSImpl::send_event ( fhg::com::p2p::address_t const& destination
                          , sdpa::events::SDPAEvent *e
                          )
{
  send_event(destination, sdpa::events::SDPAEvent::Ptr(e));
}

void DRTSImpl::send_event ( fhg::com::p2p::address_t const& destination
                          , sdpa::events::SDPAEvent::Ptr const & evt
                          )
{
  static sdpa::events::Codec codec;
  m_peer->send (destination, codec.encode(evt.get()));
}

void DRTSImpl::dispatch_event
  (fhg::com::p2p::address_t const& source, sdpa::events::SDPAEvent::Ptr const &evt)
{
  if (evt)
  {
    m_event_queue.put (source, evt);
  }
  else
  {
    LLOG (WARN, _logger, "got invalid message from suspicious source");
  }
}

DRTSImpl::Job::Job( Job::ID const &jobid
                  , Job::Description const &description
                  , owner_type const& owner
                  )
  : m_id (jobid.value)
  , m_input_description (description.value)
  , m_owner (owner)
  , m_state (Job::PENDING)
  , m_result ()
  , m_message ("")
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
