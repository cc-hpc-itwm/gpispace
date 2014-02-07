#include <drts/worker/drts.hpp>

#include <fhg/error_codes.hpp>
//! \todo remove when redoing ctor
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/threadname.hpp>

#include <gspc/net/frame_builder.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server/default_service_demux.hpp>
#include <gspc/net/service/echo.hpp>

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

#include <we/loader/module_call.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.hpp>
#include <we/type/net.hpp>

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

    wfe_exec_context (we::loader::loader& module_loader, wfe_task_t& target)
      : loader (module_loader)
      , task (target)
    {}

    virtual void handle_internally (we::type::activity_t& act, net_t const&)
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

    virtual void handle_internally (we::type::activity_t& act, mod_t const& mod)
    {
      try
      {
        we::loader::module_call (loader, &task.context, act, mod);
      }
      catch (std::exception const &ex)
      {
        throw std::runtime_error
          ( "call to '" + mod.module() + "::" + mod.function() + "'"
          + " failed: " + ex.what()
          );
      }
    }

    virtual void handle_internally (we::type::activity_t&, expr_t const&)
    {
    }

    virtual void handle_externally (we::type::activity_t& act, net_t const& n)
    {
      handle_internally (act, n);
    }

    virtual void handle_externally (we::type::activity_t& act, mod_t const& module_call)
    {
      handle_internally (act, module_call);
    }

    virtual void handle_externally (we::type::activity_t& act, expr_t const& e)
    {
      handle_internally (act, e);
    }

  private:
    we::loader::loader& loader;
    wfe_task_t& task;
  };

  struct search_path_appender
  {
    explicit search_path_appender(we::loader::loader& ld)
      : loader (ld)
    {}

    search_path_appender& operator = (std::string const& p)
    {
      if (not p.empty ())
        loader.append_search_path (p);
      return *this;
    }

    search_path_appender& operator* ()
    {
      return *this;
    }

    search_path_appender& operator++(int)
    {
      return *this;
    }

    we::loader::loader & loader;
  };
}

WFEImpl::WFEImpl ( boost::optional<std::size_t> target_socket
                 , std::string search_path
                 , boost::optional<std::string> gui_url
                 , std::string worker_name
                 , gspc::net::server::service_demux_t& service_demux
                 )
  : _numa_socket_setter ( target_socket
                        ? numa_socket_setter (*target_socket)
                        : boost::optional<numa_socket_setter>()
                        )
  , _worker_name (worker_name)
  , m_task_map()
  , m_tasks()
  , m_current_task (NULL)
  , m_loader()
  , _notification_service ( gui_url
                          ? sdpa::daemon::NotificationService (*gui_url)
                          : boost::optional<sdpa::daemon::NotificationService>()
                          )
  , _current_job_service
    ( "/service/drts/current-job"
    , boost::bind (&WFEImpl::service_current_job, this, _1, _2, _3)
    , service_demux
    )
  , _set_search_path_service
    ( "/service/drts/search-path/set"
    , boost::bind (&WFEImpl::service_set_search_path, this, _1, _2, _3)
    , service_demux
    )
  , _get_search_path_service
    ( "/service/drts/search-path/get"
    , boost::bind (&WFEImpl::service_get_search_path, this, _1, _2, _3)
    , service_demux
    )
{
  {
    // initalize loader with paths
    search_path_appender appender(m_loader);
    fhg::util::split(search_path, ":", appender);

    if (search_path.empty())
    {
      MLOG(WARN, "loader has an empty search path, try setting environment variable PC_LIBRARY_PATH or variable plugin.drts.library_path");
    }
    else
    {
      DMLOG ( DEBUG
            , "initialized loader with search path: " << search_path
            );
    }


    // TODO: figure out, why this doesn't work as it is supposed to
    // adjust ld_library_path
    std::string ld_library_path (fhg::util::getenv("LD_LIBRARY_PATH", ""));
    ld_library_path = search_path + ":" + ld_library_path;
    setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), true);
  }
}

WFEImpl::~WFEImpl()
{
  {
    boost::mutex::scoped_lock task_map_lock (m_mutex);
    while (! m_task_map.empty ())
    {
      wfe_task_t *task = m_task_map.begin ()->second;
      task->state = wfe_task_t::CANCELED;
      task->error_message = "plugin shutdown";

      m_task_map.erase (task->id);
    }
  }
}

void WFEImpl::emit_task ( const wfe_task_t& task
                        , sdpa::daemon::NotificationEvent::state_t state
                        )
{
  if (_notification_service)
  {
    _notification_service->notify
      ( sdpa::daemon::NotificationEvent
        (_worker_name, task.id, state, task.activity)
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
  task.errc = fhg::error::NO_ERROR;

  try
  {
    task.activity = we::type::activity_t (job_description);
  }
  catch (std::exception const & ex)
  {
    MLOG(ERROR, "could not parse activity: " << ex.what());
    task.state = wfe_task_t::FAILED;
    error_message = ex.what();

    emit_task (task, sdpa::daemon::NotificationEvent::STATE_FAILED);

    return fhg::error::INVALID_JOB_DESCRIPTION;
  }

  {
    boost::mutex::scoped_lock task_map_lock(m_mutex);
    m_task_map.insert(std::make_pair(job_id, &task));
  }

  {
    boost::mutex::scoped_lock lock (m_current_task_mutex);
    m_current_task = &task;
  }

  emit_task (task, sdpa::daemon::NotificationEvent::STATE_STARTED);

  if (task.state != wfe_task_t::PENDING)
  {
    task.errc = fhg::error::EXECUTION_CANCELED;
    task.error_message = "canceled";
  }
  else
  {
    try
    {
      wfe_exec_context ctxt (m_loader, task);

      task.activity.execute (&ctxt);

      if (task.state == wfe_task_t::CANCELED)
      {
        task.errc = fhg::error::EXECUTION_CANCELED;
        task.error_message = "canceled";
      }
      else
      {
        task.state = wfe_task_t::FINISHED;
        task.errc = fhg::error::NO_ERROR;
        task.error_message = "success";
      }
    }
    catch (std::exception const & ex)
    {
      task.state = wfe_task_t::FAILED;
      // TODO: more detailed error codes
      task.errc = fhg::error::MODULE_CALL_FAILED;
      task.error_message = ex.what();
    }
    catch (...)
    {
      task.state = wfe_task_t::FAILED;
      task.errc = fhg::error::UNEXPECTED_ERROR;
      task.error_message =
        "UNKNOWN REASON, exception not derived from std::exception";
    }
  }

  {
    boost::mutex::scoped_lock lock (m_current_task_mutex);
    m_current_task = 0;
  }

  result = task.activity;

  if (fhg::error::NO_ERROR == task.errc)
  {
    MLOG(TRACE, "task finished: " << task.id);
    task.state = wfe_task_t::FINISHED;
    error_message = task.error_message;

    emit_task (task, sdpa::daemon::NotificationEvent::STATE_FINISHED);
  }
  else if (fhg::error::EXECUTION_CANCELED == task.errc)
  {
    DMLOG (TRACE, "task canceled: " << task.id << ": " << task.error_message);
    task.state = wfe_task_t::CANCELED;
    error_message = task.error_message;

    emit_task (task, sdpa::daemon::NotificationEvent::STATE_CANCELED);
  }
  else
  {
    MLOG (ERROR, "task failed: " << task.id << ": " << task.error_message);
    task.state = wfe_task_t::FAILED;
    error_message = task.error_message;

    emit_task (task, sdpa::daemon::NotificationEvent::STATE_FAILED);
  }

  {
    boost::mutex::scoped_lock task_map_lock(m_mutex);
    m_task_map.erase (job_id);
  }

  return task.errc;
}

int WFEImpl::cancel (std::string const &job_id)
{
  boost::mutex::scoped_lock job_map_lock(m_mutex);
  std::map<std::string, wfe_task_t *>::iterator task_it (m_task_map.find(job_id));
  if (task_it == m_task_map.end())
  {
    MLOG(WARN, "could not find task " << job_id);
    return -ESRCH;
  }
  else
  {
    task_it->second->state = wfe_task_t::CANCELED;
    task_it->second->context.module_call_do_cancel();
  }

  return 0;
}

void WFEImpl::service_current_job ( std::string const &
                                  , gspc::net::frame const &rqst
                                  , gspc::net::user_ptr user
                                  )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  {
    boost::mutex::scoped_lock lock (m_current_task_mutex);
    if (m_current_task)
    {
      rply.set_body (m_current_task->activity.transition().name());
    }
  }

  user->deliver (rply);
}

void WFEImpl::service_get_search_path ( std::string const &
                                      , gspc::net::frame const &rqst
                                      , gspc::net::user_ptr user
                                      )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  {
    //! \todo is that lock needed really? what does it lock?
    boost::mutex::scoped_lock lock (m_mutex);

    rply.set_body (m_loader.search_path());
  }

  user->deliver (rply);
}

void WFEImpl::service_set_search_path ( std::string const &
                                      , gspc::net::frame const &rqst
                                      , gspc::net::user_ptr user
                                      )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  {
    std::string search_path (rqst.get_body ());

    search_path_appender appender(m_loader);
    boost::mutex::scoped_lock lock (m_mutex);
    m_loader.clear_search_path ();
    fhg::util::split(search_path, ":", appender);
  }

  user->deliver (rply);
}

DRTSImpl::DRTSImpl (boost::function<void()> request_stop, std::map<std::string, std::string> config_variables)
  : _net_initializer (get<std::size_t> ("plugin.drts.netd_nthreads", config_variables).get_value_or (4L))
  , _service_demux (gspc::net::server::default_service_demux())
  , _queue_manager (_service_demux)
{
  //! \todo ctor parameters
  const std::string name (*get<std::string> ("kernel_name", config_variables));
  const std::size_t backlog_size
    (get<std::size_t> ("plugin.drts.backlog", config_variables).get_value_or (3));
  const std::size_t max_reconnect_attempts
    (get<std::size_t> ("plugin.drts.max_reconnect_attempts", config_variables).get_value_or (0));
  std::list<std::string> master_list;
  std::list<std::string> capability_list;
  const boost::optional<std::size_t> target_socket
    (get<std::size_t> ("plugin.drts.socket", config_variables));
  const std::string search_path
    (get<std::string> ("plugin.drts.library_path", config_variables).get_value_or (fhg::util::getenv("PC_LIBRARY_PATH")));
  const boost::optional<std::string> gui_url
    (get<std::string> ("plugin.drts.gui_url", config_variables));
  WFEImpl* wfe (new WFEImpl (target_socket, search_path, gui_url, name, _service_demux));
  fhg::com::host_t host (get<std::string> ("plugin.drts.host", config_variables).get_value_or ("*"));
  fhg::com::port_t port (get<std::string> ("plugin.drts.port", config_variables).get_value_or ("0"));
  {
    const std::string master_names (get<std::string> ("plugin.drts.master", config_variables).get_value_or (""));
    const std::string virtual_capabilities (get<std::string> ("plugin.drts.capabilities", config_variables).get_value_or (""));
    fhg::util::split (master_names, ",", std::back_inserter(master_list));
    fhg::util::split (virtual_capabilities, ",", std::back_inserter(capability_list));
  }
  const std::string netd_url (get<std::string> ("plugin.drts.netd_url", config_variables).get_value_or ("tcp://*"));

  const std::string kvs_host (get<std::string> ("plugin.drts.kvs_host", config_variables).get_value_or ("localhost"));
  const std::string kvs_port (get<std::string> ("plugin.drts.kvs_port", config_variables).get_value_or ("2439"));
  const unsigned int kvs_max_ping_failed (get<unsigned int> ("plugin.drts.kvs_max_ping_failed", config_variables).get_value_or (3));
  const boost::posix_time::time_duration kvs_ping_interval
    ( boost::posix_time::duration_from_string
      ( get<std::string> ("plugin.drts.kvs_ping", config_variables)
      .get_value_or
        (boost::posix_time::to_simple_string (boost::posix_time::seconds (5)))
      )
    );
  const boost::posix_time::time_duration kvs_timeout
    ( boost::posix_time::duration_from_string
      ( get<std::string> ("plugin.drts.kvs_timeout", config_variables)
      .get_value_or
        (boost::posix_time::to_simple_string (boost::posix_time::seconds (120)))
      )
    );


  _request_stop = request_stop;

  fhg::com::kvs::get_or_create_global_kvs
    ( !kvs_host.empty() ? kvs_host : throw std::runtime_error ("kvs host empty")
    , !kvs_port.empty() ? kvs_port : throw std::runtime_error ("kvs port empty")
    , true // auto_reconnect
    , kvs_timeout
    , 1 // max_connection_attempts
    );

  _kvs_keep_alive = new fhg::util::keep_alive
    ( boost::bind (&fhg::com::kvs::client::kvsc::ping, fhg::com::kvs::global_kvs())
    , _request_stop
    , kvs_max_ping_failed
    , kvs_ping_interval
    );

  _service_demux.handle ("/service/echo", gspc::net::service::echo ());

  m_server = gspc::net::serve (netd_url, _queue_manager);

  fhg::com::kvs::global_kvs()->put ("gspc.net.url." + name, m_server->url());


  m_shutting_down = false;

  m_reconnect_counter = 0;
  m_my_name = name;
  m_backlog_size = backlog_size;
  m_max_reconnect_attempts = max_reconnect_attempts;
  m_wfe = wfe;

  // parse virtual capabilities
  BOOST_FOREACH (std::string const & cap, capability_list)
  {
    if (m_virtual_capabilities.find(cap) == m_virtual_capabilities.end())
    {
      std::pair<std::string, std::string> capability_and_type
        = fhg::util::split_string (cap, "-");
      if (capability_and_type.second.empty ())
        capability_and_type.second = "virtual";

      const std::string & cap_name = capability_and_type.first;
      const std::string & cap_type = capability_and_type.second;

      DMLOG ( TRACE
            , "adding capability: " << cap_name
            << " of type: " << cap_type
            );

      m_virtual_capabilities.insert
        (std::make_pair (cap, sdpa::Capability (cap_name, cap_type, m_my_name)));
    }
  }

  assert (! m_event_thread);

  m_event_thread.reset(new boost::thread(&DRTSImpl::event_thread, this));
  fhg::util::set_threadname (*m_event_thread, "[drts-events]");

  // initialize peer
  m_peer.reset (new fhg::com::peer_t (m_my_name, host, port, fhg::com::kvs::global_kvs()));
  m_peer_thread.reset(new boost::thread(&fhg::com::peer_t::run, m_peer));
  fhg::util::set_threadname (*m_peer_thread, "[drts-peer]");
  m_peer->start();

  start_receiver();

  if (master_list.empty())
  {
    throw std::runtime_error ("no masters specified");
  }

  BOOST_FOREACH (std::string const & master, master_list)
  {
    if (m_masters.find (master) == m_masters.end ())
    {
      DMLOG(TRACE, "adding master \"" << master << "\"");

      if (master.empty())
      {
        throw std::runtime_error ("empty master specified!");
      }

      if (master == m_my_name)
      {
        throw std::runtime_error ("cannot be my own master!");
      }

      m_masters.insert (std::make_pair(master, false));
    }
    else
    {
      MLOG( WARN
          , "master already specified, ignoring new one: " << master
          );
    }
  }

  m_execution_thread.reset
    (new boost::thread(&DRTSImpl::job_execution_thread, this));
  fhg::util::set_threadname (*m_execution_thread, "[drts-execute]");

  start_connect ();

  _service_demux.handle
    ("/service/drts/capability/add"
    , boost::bind (&DRTSImpl::service_capability_add, this, _1, _2, _3)
    );

  _service_demux.handle
    ("/service/drts/capability/del"
    , boost::bind (&DRTSImpl::service_capability_del, this, _1, _2, _3)
    );

  _service_demux.handle
    ("/service/drts/capability/get"
    , boost::bind (&DRTSImpl::service_capability_get, this, _1, _2, _3)
    );
}

DRTSImpl::~DRTSImpl()
{
  _service_demux.unhandle ("/service/drts/capability/add");
  _service_demux.unhandle ("/service/drts/capability/del");
  _service_demux.unhandle ("/service/drts/capability/get");

  m_shutting_down = true;

  if (m_execution_thread)
  {
    m_execution_thread->interrupt();
    if (m_execution_thread->joinable ())
      m_execution_thread->join ();
    m_execution_thread.reset();
  }

  if (m_event_thread)
  {
    m_event_thread->interrupt();
    if (m_event_thread->joinable ())
      m_event_thread->join();
    m_event_thread.reset();
  }

  if (m_peer)
  {
    m_peer->stop();
  }

  if (m_peer_thread)
  {
    m_peer_thread->interrupt();
    if (m_peer_thread->joinable ())
      m_peer_thread->join();
  }

  m_peer_thread.reset();
  m_peer.reset();

  delete m_wfe;
  m_wfe = 0;

  if (m_server)
  {
    m_server->stop ();
  }

  delete _kvs_keep_alive;
  _kvs_keep_alive = NULL;
}

  // event handler callbacks
  //    implemented events
void DRTSImpl::handleWorkerRegistrationAckEvent
  (const sdpa::events::WorkerRegistrationAckEvent *e)
{
  map_of_masters_t::iterator master_it (m_masters.find(e->from()));
  if (master_it != m_masters.end())
  {
    if (!master_it->second)
    {
      DMLOG(TRACE, "successfully connected to " << master_it->first);
      master_it->second = true;

      notify_capabilities_to_master (master_it->first);
      resend_outstanding_events (master_it->first);

      m_connected_event.notify(master_it->first);
    }

    {
      boost::mutex::scoped_lock lock_reconnect_counter (m_reconnect_counter_mutex);
      m_reconnect_counter = 0;
    }
  }
}
void DRTSImpl::handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e)
{
  MLOG(WARN, "worker tried to register: " << e->from());

  send_event
    (new sdpa::events::ErrorEvent( m_my_name
                                 , e->from()
                                 , sdpa::events::ErrorEvent::SDPA_EPERM
                                 , "you are not allowed to connect"
                                 )
    );
}

void DRTSImpl::handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*)
{
}

void DRTSImpl::handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*)
{
}

void DRTSImpl::handleDeleteJobEvent(const sdpa::events::DeleteJobEvent *)
{
}

void DRTSImpl::handleErrorEvent(const sdpa::events::ErrorEvent *)
{
}

void DRTSImpl::handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent *)
{
}

void DRTSImpl::handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent *)
{
}

void DRTSImpl::handleSubmitJobEvent(const sdpa::events::SubmitJobEvent *e)
{
  // check master
  map_of_masters_t::const_iterator master (m_masters.find(e->from()));

  if (master == m_masters.end())
  {
    MLOG(ERROR, "got SubmitJob from unknown source: " << e->from());
    return;
  }
  else if (! master->second)
  {
    MLOG(WARN, "got SubmitJob from not yet connected master: " << e->from());
    send_event
      (new sdpa::events::ErrorEvent( m_my_name
                                   , e->from()
                                   , sdpa::events::ErrorEvent::SDPA_EPERM
                                   , "you are not yet connected"
                                   , *e->job_id()
                                   )
      );
    return;
  }

  boost::shared_ptr<drts::Job> job (new drts::Job( drts::Job::ID(*e->job_id())
                                                 , drts::Job::Description(e->description())
                                                 , drts::Job::Owner(e->from())
                                                 )
                                   );

  job->worker_list (e->worker_list ());

  {
    boost::mutex::scoped_lock job_map_lock(m_job_map_mutex);

    if (m_backlog_size && m_pending_jobs.size() >= m_backlog_size)
    {
      MLOG( WARN
          , "cannot accept new job (" << job->id() << "), backlog is full."
          );
      send_event (new sdpa::events::ErrorEvent
                   ( m_my_name
                   , e->from()
                   , sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                   , "I am busy right now, please try again later!"
                   , *e->job_id()
                   ));
      return;
    }
    else
    {
    	DLOG( INFO, "Worker "<<m_my_name<<": received from "
    			             <<e->from()<<" the job " << job->id()
    	   			  	     <<", assigned to "<<e->worker_list() );

      send_event (new sdpa::events::SubmitJobAckEvent( m_my_name
                                                     , job->owner()
                                                     , job->id()
                                                     )
                 );
      m_jobs.insert (std::make_pair(job->id(), job));

      m_pending_jobs.put(job);
    }
  }

  //    boost::mutex::scoped_lock lock(m_job_arrived_mutex);
  m_job_arrived.notify_all();
}

void DRTSImpl::handleCancelJobEvent(const sdpa::events::CancelJobEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  MLOG(TRACE, "got cancelation request for job: " << e->job_id());

  if (job_it == m_jobs.end())
  {
    DMLOG (WARN, "could not cancel job: " << e->job_id() << ": not found");
    send_event(new sdpa::events::ErrorEvent
                ( m_my_name
                , e->from()
                , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                , "could not find job " + std::string(e->job_id())
                ));
  }
  else if (job_it->second->owner() != e->from())
  {
    DMLOG (ERROR, "could not cancel job: " << e->job_id() << ": not owner");
    send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }
  else
  {
    if (  drts::Job::PENDING
       == job_it->second->cmp_and_swp_state( drts::Job::PENDING
                                           , drts::Job::CANCELED
                                           )
       )
    {
      MLOG(TRACE, "canceling pending job: " << e->job_id());
      send_event
        (new sdpa::events::CancelJobAckEvent ( m_my_name
                                             , job_it->second->owner()
                                             , job_it->second->id()
                                             )
        );
    }
    else if (job_it->second->state() == drts::Job::RUNNING)
    {
      MLOG (TRACE, "trying to cancel running job " << e->job_id());
      m_wfe->cancel (e->job_id());
    }
    else if (job_it->second->state() == drts::Job::FAILED)
    {
      MLOG(TRACE, "canceling already failed job: " << e->job_id());
    }
    else if (job_it->second->state() == drts::Job::CANCELED)
    {
      MLOG(TRACE, "canceling already canceled job: " << e->job_id());
    }
    else
    {
      MLOG( WARN
          , "what shall I do with an already computed job? "
          << "(" << e->job_id() << ")"
          );
    }
  }
}

void DRTSImpl::handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
  if (job_it == m_jobs.end())
  {
    MLOG( ERROR
        , "could not acknowledge failed job: " << e->job_id() << ": not found"
        );
    send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (job_it->second->owner() != e->from())
  {
    MLOG( ERROR
        , "could not acknowledge failed job: " << e->job_id() << ": not owner"
        );
    send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }

  DMLOG(TRACE, "removing job " << e->job_id());
  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
  if (job_it == m_jobs.end())
  {
    MLOG( ERROR
        , "could not acknowledge finished job: " << e->job_id()
        << ": not found"
        );
    send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (job_it->second->owner() != e->from())
  {
    MLOG( ERROR
        , "could not acknowledge finished job: " << e->job_id()
        << ": not owner"
        );
    send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }

  DMLOG(TRACE, "removing job " << e->job_id());
  m_jobs.erase (job_it);
}

  // not implemented events
void DRTSImpl::handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *){}
void DRTSImpl::handleJobFailedEvent(const sdpa::events::JobFailedEvent *) {}
void DRTSImpl::handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *) {}
void DRTSImpl::handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *) {}
void DRTSImpl::handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *) {}
void DRTSImpl::handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent *) {}

  // threads
void DRTSImpl::event_thread ()
{
  for (;;)
  {
    try
    {
      sdpa::events::SDPAEvent::Ptr evt(m_event_queue.get());
      evt->handleBy(this);
    }
    catch (boost::thread_interrupted const & irq)
    {
      DMLOG(TRACE, "event handler interrupted...");
      throw;
    }
    catch (std::exception const & ex)
    {
      MLOG(WARN, "event could not be handled: " << ex.what());
    }
  }
}

void DRTSImpl::job_execution_thread ()
{
  for (;;)
  {
    boost::shared_ptr<drts::Job> job = m_pending_jobs.get();

    if (drts::Job::PENDING == job->cmp_and_swp_state( drts::Job::PENDING
                                                    , drts::Job::RUNNING
                                                    )
       )
    {
      try
      {
        const boost::posix_time::ptime started
          (boost::posix_time::microsec_clock::universal_time());

        MLOG(TRACE, "executing job " << job->id());

        we::type::activity_t result;
        std::string error_message;
        int ec = m_wfe->execute ( job->id()
                                , job->description()
                                , result
                                , error_message
                                , job->worker_list ()
                                );
        job->set_result (result.to_string());
        job->set_result_code (ec);
        job->set_message (error_message);

        const boost::posix_time::ptime completed
          (boost::posix_time::microsec_clock::universal_time());

        MLOG( TRACE
            , "job returned."
            << " error-code := " << job->result_code()
            << " error-message := " << job->message()
            << " total-time := " << (completed - started)
            );

        if (fhg::error::NO_ERROR == ec)
        {
          job->set_state (drts::Job::FINISHED);
        }
        else if (fhg::error::EXECUTION_CANCELED == ec)
        {
          job->set_state (drts::Job::CANCELED);
        }
        else
        {
          job->set_state (drts::Job::FAILED);
        }
      }
      catch (std::exception const & ex)
      {
        MLOG( ERROR
            , "unexpected exception during job execution: " << ex.what()
            );
        job->set_state (drts::Job::FAILED);

        job->set_result (job->description());
        job->set_result_code (fhg::error::UNEXPECTED_ERROR);
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
        DMLOG(TRACE, "ignoring and erasing non-pending job " << job->id());
        m_jobs.erase(job_it);
      }
    }
  }
}

void DRTSImpl::add_virtual_capability (std::string const &cap)
{
  boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

  if (m_virtual_capabilities.find(cap) == m_virtual_capabilities.end())
  {
    DMLOG(TRACE, "adding virtual capability: " << cap);
    m_virtual_capabilities.insert
      (std::make_pair (cap, sdpa::Capability (cap, "virtual", m_my_name)));

    notify_capability_gained (m_virtual_capabilities[cap]);
  }
}

void DRTSImpl::del_virtual_capability (std::string const &cap)
{
  boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

  typedef map_of_capabilities_t::iterator cap_it_t;
  cap_it_t cap_it = m_virtual_capabilities.find (cap);
  if (cap_it != m_virtual_capabilities.end ())
  {
    notify_capability_lost (cap_it->second);

    m_virtual_capabilities.erase (cap);
  }
}

void DRTSImpl::service_capability_add ( std::string const &
                                      , gspc::net::frame const &rqst
                                      , gspc::net::user_ptr user
                                      )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  std::string virtual_capabilities (rqst.get_body ());
  std::list<std::string> capability_list;
  fhg::util::split( virtual_capabilities
                  , ","
                  , std::back_inserter(capability_list)
                  );

  BOOST_FOREACH (std::string const & cap, capability_list)
  {
    add_virtual_capability (cap);
  }

    user->deliver (rply);
}

void DRTSImpl::service_capability_del ( std::string const &
                                      , gspc::net::frame const &rqst
                                      , gspc::net::user_ptr user
                                      )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  std::string virtual_capabilities (rqst.get_body ());
  std::list<std::string> capability_list;
  fhg::util::split( virtual_capabilities
                  , ","
                  , std::back_inserter(capability_list)
                  );

  BOOST_FOREACH (std::string const & cap, capability_list)
  {
    del_virtual_capability (cap);
  }

  user->deliver (rply);
}

void DRTSImpl::service_capability_get ( std::string const &
                                      , gspc::net::frame const &rqst
                                      , gspc::net::user_ptr user
                                      )
{
  boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  typedef map_of_capabilities_t::const_iterator const_cap_it_t;
  for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
      ; cap_it != m_virtual_capabilities.end()
      ; ++cap_it
      )
  {
    rply.add_body (cap_it->first + "\n");
  }

  user->deliver (rply);
}

void DRTSImpl::notify_capabilities_to_master (std::string const &master)
{
  sdpa::capabilities_set_t caps;
  boost::mutex::scoped_lock capabilities_lock(m_capabilities_mutex);

  typedef map_of_capabilities_t::const_iterator const_cap_it_t;
  for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
      ; cap_it != m_virtual_capabilities.end()
      ; ++cap_it
      )
  {
    caps.insert (cap_it->second);
  }

  if (! caps.empty())
  {
    send_event(new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                        , master
                                                        , caps
                                                        )
              );
  }
}

void DRTSImpl::notify_capability_gained (sdpa::Capability const &cap)
{
  for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
      ; master_it != m_masters.end()
      ; ++master_it
      )
  {
    if (not master_it->second)
      continue;

    send_event
      (new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                , master_it->first
                                                , cap
                                                ));
  }
}

void DRTSImpl::notify_capability_lost (sdpa::Capability const &cap)
{
  for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
      ; master_it != m_masters.end()
      ; ++master_it
      )
  {
    if (not master_it->second)
      continue;

    send_event
      (new sdpa::events::CapabilitiesLostEvent( m_my_name
                                              , master_it->first
                                              , cap
                                              )
      );
  }
}

void DRTSImpl::resend_outstanding_events (std::string const &master)
{
  MLOG(TRACE, "resending outstanding notifications to " << master);
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  for ( map_of_jobs_t::iterator job_it (m_jobs.begin()), end (m_jobs.end())
      ; job_it != end
      ; ++job_it
      )
  {
    boost::shared_ptr<drts::Job> job (job_it->second);
    MLOG( TRACE
        , "checking job"
        << " id := " << job->id()
        << " state := " << job->state()
        << " owner := " << job->owner()
        );
    if (   (job->owner() == master)
       && (job->state() >= drts::Job::FINISHED)
       )
    {
      MLOG(TRACE, "resending outcome of job " << job->id());
      send_job_result_to_master (job);
    }
  }
}

void DRTSImpl::send_job_result_to_master (boost::shared_ptr<drts::Job> const & job)
{
  switch (job->state())
  {
  case drts::Job::FINISHED:
    send_event (new sdpa::events::JobFinishedEvent ( m_my_name
                                                   , job->owner()
                                                   , job->id()
                                                   , job->result()
                                                   )
               );
    break;
  case drts::Job::FAILED:
    {
      send_event
        (new sdpa::events::JobFailedEvent ( m_my_name
                                          , job->owner()
                                          , job->id()
                                          , job->result_code()
                                          , job->message()
                                          )
        );
    }
    break;
  case drts::Job::CANCELED:
    {
      send_event
        (new sdpa::events::CancelJobAckEvent ( m_my_name
                                             , job->owner()
                                             , job->id()
                                             )
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
    (boost::bind (&DRTSImpl::request_registration_after_sleep, this));
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
    if (! master_it->second)
    {
      sdpa::events::WorkerRegistrationEvent::Ptr evt
        (new sdpa::events::WorkerRegistrationEvent( m_my_name
                                                  , master_it->first
                                                  , m_backlog_size
                                                  )
        );

      send_event(evt);

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
        MLOG( WARN
            , "still not connected after " << m_reconnect_counter
            << " trials: shutting down"
            );
        _request_stop();
        return;
      }
    }
  }


  if (at_least_one_disconnected)
  {
    request_registration_soon();
  }
}

void DRTSImpl::start_receiver()
{
  m_peer->async_recv(&m_message, boost::bind( &DRTSImpl::handle_recv
                                            , this
                                            , _1
                                            )
                    );
}

void DRTSImpl::handle_recv (boost::system::error_code const & ec)
{
  static sdpa::events::Codec codec;

  if (! ec)
  {
    // convert m_message to event
    try
    {
      dispatch_event
        (sdpa::events::SDPAEvent::Ptr
          (codec.decode (std::string ( m_message.data.begin()
                                     , m_message.data.end()
                                     )
                        )
          ));
    }
    catch (std::exception const & ex)
    {
      MLOG(WARN, "could not handle incoming message: " << ex.what());
    }
    start_receiver();
  }
  else if (! m_shutting_down)
  {
    const fhg::com::p2p::address_t & addr = m_message.header.src;
    if (addr != m_peer->address())
    {
      const std::string other_name(m_peer->resolve (addr, "*unknown*"));

      map_of_masters_t::iterator master(m_masters.find(other_name));
      if (master != m_masters.end() && master->second)
      {
        DMLOG ( INFO
              , "connection to " << other_name << " lost: " << ec.message()
              );

        master->second = false;

        request_registration_soon();
      }

      start_receiver();
    }
    else
    {
      MLOG(TRACE, m_peer->name() << " is shutting down");
    }
  }
}

void DRTSImpl::send_event (sdpa::events::SDPAEvent *e)
{
  send_event(sdpa::events::SDPAEvent::Ptr(e));
}

void DRTSImpl::send_event (sdpa::events::SDPAEvent::Ptr const & evt)
{
  static sdpa::events::Codec codec;

  const std::string encoded_evt (codec.encode(evt.get()));

  m_peer->send (evt->to(), encoded_evt);
}

void DRTSImpl::dispatch_event (sdpa::events::SDPAEvent::Ptr const &evt)
{
  if (evt)
  {
    DMLOG(TRACE, "received event: " << evt->str());
    m_event_queue.put(evt);
  }
  else
  {
    MLOG(WARN, "got invalid message from suspicious source");
  }
}
