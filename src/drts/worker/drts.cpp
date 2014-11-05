#include <drts/worker/drts.hpp>

//! \todo remove when redoing ctor
#include <plugin/plugin.hpp>
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

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <functional>

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
    wfe_task_t& task;
  };
}

WFEImpl::WFEImpl ( fhg::log::Logger::ptr_t logger
                 , boost::optional<std::size_t> target_socket
                 , std::string search_path
                 , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
                 , std::string worker_name
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

    return drts::Job::FAILED;
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
      wfe_exec_context ctxt (m_loader, task);

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

  return task.state == wfe_task_t::FINISHED ? drts::Job::FINISHED
    : task.state == wfe_task_t::CANCELED ? drts::Job::CANCELED
    : task.state == wfe_task_t::FAILED ? drts::Job::FAILED
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


DRTSImpl::DRTSImpl
    ( std::function<void()> request_stop
    , boost::asio::io_service& peer_io_service
    , boost::asio::io_service& kvs_client_io_service
    , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
    , std::map<std::string, std::string> config_variables
    )
  : _logger
    (fhg::log::Logger::get (*get<std::string> ("kernel_name", config_variables)))
  , _request_stop (request_stop)
  , _kvs_client
    (new fhg::com::kvs::client::kvsc
      ( kvs_client_io_service
      , *get<std::string> ("plugin.drts.kvs_host", config_variables)
      , *get<std::string> ("plugin.drts.kvs_port", config_variables)
      , true // auto_reconnect
      , boost::posix_time::duration_from_string
        ( get<std::string> ("plugin.drts.kvs_timeout", config_variables)
        .get_value_or
          (boost::posix_time::to_simple_string (boost::posix_time::seconds (120)))
        )
      , 1 // max_connection_attempts
      )
    )
  , m_shutting_down (false)
  , m_my_name (*get<std::string> ("kernel_name", config_variables))
  , m_wfe ( _logger
          , get<std::size_t> ("plugin.drts.socket", config_variables)
          , get<std::string> ("plugin.drts.library_path", config_variables).get_value_or (fhg::util::getenv("PC_LIBRARY_PATH").get_value_or (""))
          , gui_info
          , m_my_name
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
    ( new fhg::com::peer_t
      ( peer_io_service
      , m_my_name
      , host
      , port
      , _kvs_client
      , [](boost::system::error_code const&)
      {
        MLOG (ERROR, "could not contact KVS...");
      }
      )
    );
  m_peer_thread.reset
    ( new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      (&fhg::com::peer_t::run, m_peer)
    );
  m_peer->start();

  start_receiver();

  for (std::string const & master : master_list)
  {
    if (master.empty())
    {
      throw std::runtime_error ("empty master specified!");
    }

    if (master == m_my_name)
    {
      throw std::runtime_error ("cannot be my own master!");
    }

    if (!m_masters.emplace (master, boost::none).second)
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
  (std::string const& source, const sdpa::events::WorkerRegistrationAckEvent*)
{
  fhg::com::p2p::address_t const source_address (source);
  map_of_masters_t::const_iterator master_it
    ( std::find_if ( m_masters.cbegin(), m_masters.cend()
                   , [&] (map_of_masters_t::value_type const& master)
                     {
                       return master.second == source_address;
                     }
                   )
    );
  if (master_it != m_masters.end())
  {
    {
      boost::mutex::scoped_lock const _ (m_capabilities_mutex);

      if (!m_virtual_capabilities.empty())
      {
        send_event ( source_address
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
  (std::string const& source, const sdpa::events::SubmitJobEvent *e)
{
  // check master
  map_of_masters_t::const_iterator master (m_masters.find(source));

  if (master == m_masters.end())
  {
    throw std::runtime_error ("got SubmitJob from unknown source");
  }
  else if (! master->second)
  {
    throw std::runtime_error ("got SubmitJob from not yet connected master");
  }

  boost::shared_ptr<drts::Job> job (new drts::Job( drts::Job::ID(*e->job_id())
                                                 , drts::Job::Description(e->description())
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
      send_event ( fhg::com::p2p::address_t (source)
                 , new sdpa::events::ErrorEvent
                   ( sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                   , "I am busy right now, please try again later!"
                   , *e->job_id()
                   ));
      return;
    }
    else
    {
      send_event (*master->second, new sdpa::events::SubmitJobAckEvent (job->id()));
      m_jobs.emplace (job->id(), job);

      m_pending_jobs.put(job);
    }
  }

  //    boost::mutex::scoped_lock lock(m_job_arrived_mutex);
  m_job_arrived.notify_all();
}

void DRTSImpl::handleCancelJobEvent
  (std::string const& source, const sdpa::events::CancelJobEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

  LLOG (TRACE, _logger, "got cancelation request for job: " << e->job_id());

  if (job_it == m_jobs.end())
  {
    send_event( fhg::com::p2p::address_t (source)
              , new sdpa::events::ErrorEvent
                ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                , "could not find job " + std::string(e->job_id())
                ));
  }
  else if (*job_it->second->owner()->second != fhg::com::p2p::address_t (source))
  {
    send_event ( fhg::com::p2p::address_t (source)
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
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
      LLOG (TRACE, _logger, "canceling pending job: " << e->job_id());
      send_event
        ( *job_it->second->owner()->second
        , new sdpa::events::CancelJobAckEvent (job_it->second->id())
        );
    }
    else if (job_it->second->state() == drts::Job::RUNNING)
    {
      LLOG (TRACE, _logger, "trying to cancel running job " << e->job_id());
      m_wfe.cancel (e->job_id());
    }
    else if (job_it->second->state() == drts::Job::FAILED)
    {
      LLOG (TRACE, _logger, "canceling already failed job: " << e->job_id());
    }
    else if (job_it->second->state() == drts::Job::CANCELED)
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
  (std::string const& source, const sdpa::events::JobFailedAckEvent *e)
{
  // locate the job
  boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
  map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
  if (job_it == m_jobs.end())
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge failed job: " << e->job_id() << ": not found"
        );
    send_event ( fhg::com::p2p::address_t (source)
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (*job_it->second->owner()->second != fhg::com::p2p::address_t (source))
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge failed job: " << e->job_id() << ": not owner"
        );
    send_event ( fhg::com::p2p::address_t (source)
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleJobFinishedAckEvent
  (std::string const& source, const sdpa::events::JobFinishedAckEvent *e)
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
    send_event ( fhg::com::p2p::address_t (source)
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                 , "could not find job " + std::string(e->job_id())
                 ));
    return;
  }
  else if (*job_it->second->owner()->second != fhg::com::p2p::address_t (source))
  {
    LLOG ( ERROR, _logger
        , "could not acknowledge finished job: " << e->job_id()
        << ": not owner"
        );
    send_event ( fhg::com::p2p::address_t (source)
               , new sdpa::events::ErrorEvent
                 ( sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
    return;
  }

  m_jobs.erase (job_it);
}

void DRTSImpl::handleDiscoverJobStatesEvent
  (std::string const& source, const sdpa::events::DiscoverJobStatesEvent* event)
{
  boost::mutex::scoped_lock const _ (m_job_map_mutex);

  const map_of_jobs_t::iterator job_it (m_jobs.find (event->job_id()));
  send_event ( fhg::com::p2p::address_t (source)
             , new sdpa::events::DiscoverJobStatesReplyEvent
               ( event->discover_id()
               , sdpa::discovery_info_t
                 ( event->job_id()
                 , job_it == m_jobs.end() ? boost::optional<sdpa::status::code>()
                 : job_it->second->state() == drts::Job::PENDING ? sdpa::status::PENDING
                 : job_it->second->state() == drts::Job::RUNNING ? sdpa::status::RUNNING
                 : job_it->second->state() == drts::Job::FINISHED ? sdpa::status::FINISHED
                 : job_it->second->state() == drts::Job::FAILED ? sdpa::status::FAILED
                 : job_it->second->state() == drts::Job::CANCELED ? sdpa::status::CANCELED
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
    std::pair<std::string, sdpa::events::SDPAEvent::Ptr> event
      (m_event_queue.get());
    event.second->handleBy (event.first, this);
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

        job->set_state (drts::Job::state_t (ec));

        if (ec == drts::Job::FAILED)
        {
          job->set_message (error_message);
        }
      }
      catch (std::exception const & ex)
      {
        LLOG ( ERROR, _logger
            , "unexpected exception during job execution: " << ex.what()
            );
        job->set_state (drts::Job::FAILED);

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

  for ( boost::shared_ptr<drts::Job> const& job
      : m_jobs
      | boost::adaptors::map_values
      | boost::adaptors::filtered
          ( [&master] (boost::shared_ptr<drts::Job> const& j)
            {
              return j->owner() == master && j->state() >= drts::Job::FINISHED;
            }
          )
      )
  {
    LLOG (TRACE, _logger, "resending outcome of job " << job->id());
    send_job_result_to_master (job);
  }
}

void DRTSImpl::send_job_result_to_master (boost::shared_ptr<drts::Job> const & job)
{
  switch (job->state())
  {
  case drts::Job::FINISHED:
    send_event ( *job->owner()->second
               , new sdpa::events::JobFinishedEvent (job->id(), job->result())
               );
    break;
  case drts::Job::FAILED:
    {
      send_event
        ( *job->owner()->second
        , new sdpa::events::JobFailedEvent (job->id(), job->message())
        );
    }
    break;
  case drts::Job::CANCELED:
    {
      send_event
        (*job->owner()->second, new sdpa::events::CancelJobAckEvent (job->id()));
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
    if (! master_it->second)
    {
      sdpa::events::WorkerRegistrationEvent::Ptr evt
        (new sdpa::events::WorkerRegistrationEvent ( m_backlog_size
                                                   , sdpa::capabilities_set_t()
                                                   , false
                                                   , fhg::util::hostname()
                                                   )
        );

      try
      {
        master_it->second = m_peer->connect_to_via_kvs (master_it->first);
        send_event(*master_it->second, evt);
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
                           , boost::optional<std::string> source_name
                           )
{
  static sdpa::events::Codec codec;

  if (! ec)
  {
    // convert m_message to event
    try
    {
      dispatch_event
        ( source_name.get()
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
    if (m_message.header.src != m_peer->address())
    {
      map_of_masters_t::iterator master(m_masters.find(source_name.get()));
      if (master != m_masters.end() && master->second)
      {
        master->second = boost::none;

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
  (std::string const& source, sdpa::events::SDPAEvent::Ptr const &evt)
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
