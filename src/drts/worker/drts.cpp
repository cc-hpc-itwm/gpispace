#include "job.hpp"

#include <fhg/error_codes.hpp>
#include <fhg/plugin/capability.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/threadname.hpp>

#include <fhgcom/peer.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/remote/appender.hpp>

#include <gspc/drts/context.hpp>
#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/server/default_queue_manager.hpp>
#include <gspc/net/server/default_service_demux.hpp>
#include <gspc/net/service/echo.hpp>
#include <gspc/net/user.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/events.hpp>
#include <sdpa/types.hpp>

#include <plugins/kvs.hpp>

#include <we/context.hpp>
#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/type/activity.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.hpp>
//! \todo eliminate this include (that completes the type transition_t::data)
#include <we/type/net.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <hwloc.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include <errno.h>

namespace
{
  struct wfe_task_t
  {
    enum state_t
    {
      PENDING
    , CANCELED
    , FINISHED
    , FAILED
    };

    std::string id;
    state_t state;
    int        errc;
    we::type::activity_t activity;
    gspc::drts::context context;
    std::string error_message;

    wfe_task_t (std::string id, std::list<std::string> workers)
      : id (id)
      , state (wfe_task_t::PENDING)
      , context (workers)
    {}
  };

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

  class numa_socket_setter
  {
  public:
    numa_socket_setter (size_t target_socket)
    {
      hwloc_topology_init (&m_topology);
      hwloc_topology_load (m_topology);

      const int depth (hwloc_get_type_depth (m_topology, HWLOC_OBJ_SOCKET));
      if (depth == HWLOC_TYPE_DEPTH_UNKNOWN)
      {
        throw std::runtime_error ("could not get number of sockets");
      }

      const size_t available_sockets
        (hwloc_get_nbobjs_by_depth (m_topology, depth));

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

    ~numa_socket_setter()
    {
      hwloc_topology_destroy (m_topology);
    }

  private:
    hwloc_topology_t m_topology;
  };

  struct scoped_service_handler : boost::noncopyable
  {
    scoped_service_handler ( std::string name
                           , boost::function<void ( std::string const &dst
                                                  , gspc::net::frame const &rqst
                                                  , gspc::net::user_ptr user
                                                  )> function
                           , gspc::net::server::service_demux_t& service_demux
                           )
      : _name (name)
      , _service_demux (service_demux)
    {
      _service_demux.handle (_name, function);
    }
    ~scoped_service_handler()
    {
      _service_demux.unhandle (_name);
    }
  private:
    std::string _name;
    gspc::net::server::service_demux_t& _service_demux;
  };
}

class WFEImpl
{
public:
  WFEImpl ( boost::optional<std::size_t> target_socket
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

  ~WFEImpl()
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

private:
  void emit_task ( const wfe_task_t& task
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

public:
  int execute ( std::string const &job_id
              , std::string const &job_description
              , we::type::activity_t & result
              , std::string & error_message
              , std::list<std::string> const & worker_list
              )
  {
    wfe_task_t task (job_id, worker_list);
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

  int cancel (std::string const &job_id)
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
private:
  void service_current_job ( std::string const &
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

  void service_get_search_path ( std::string const &
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

  void service_set_search_path ( std::string const &
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

  boost::optional<numa_socket_setter> _numa_socket_setter;

  std::string _worker_name;

  mutable boost::mutex m_mutex;
  std::map<std::string, wfe_task_t *> m_task_map;
  fhg::thread::queue<wfe_task_t*> m_tasks;

  mutable boost::mutex m_current_task_mutex;
  wfe_task_t *m_current_task;

  we::loader::loader m_loader;

  boost::optional<sdpa::daemon::NotificationService> _notification_service;

  scoped_service_handler _current_job_service;
  scoped_service_handler _set_search_path_service;
  scoped_service_handler _get_search_path_service;
};

class DRTSImpl : FHG_PLUGIN
               , public sdpa::events::EventHandler
{
  typedef std::map<std::string, bool> map_of_masters_t;

  typedef std::map< std::string
                  , boost::shared_ptr<drts::Job>
                  > map_of_jobs_t;
  typedef std::map< std::string
                  , std::pair<sdpa::Capability, fhg::plugin::Capability*>
                  > map_of_capabilities_t;
public:
  FHG_PLUGIN_START()
  try
  {
    //! \todo ctor parameters
    const std::string name (fhg_kernel()->get_name());
    const std::size_t backlog_size
      (fhg_kernel()->get<std::size_t> ("backlog", "3"));
    const bool terminate_on_failure
      (fhg::util::read_bool (fhg_kernel()->get ("terminate_on_failure", "false")));
    const std::size_t max_reconnect_attempts
      (fhg_kernel()->get<std::size_t> ("max_reconnect_attempts", "0"));
    std::list<std::string> master_list;
    std::list<std::string> capability_list;
    const std::size_t target_socket_
      (fhg_kernel()->get<std::size_t> ("socket", -1));
    const boost::optional<std::size_t> target_socket
      (boost::make_optional (target_socket_ != std::size_t (-1), target_socket_));
    const std::string search_path
      (fhg_kernel()->get("library_path", fhg::util::getenv("PC_LIBRARY_PATH")));
    const std::string gui_url_ (fhg_kernel()->get ("gui_url", ""));
    const boost::optional<std::string> gui_url
      (boost::make_optional (!gui_url_.empty(), gui_url_));
    WFEImpl* wfe (new WFEImpl (target_socket, search_path, gui_url, name, gspc::net::server::default_service_demux()));
    //! \note optional
    fhg::plugin::Capability* cap (fhg_kernel()->acquire< fhg::plugin::Capability>("gpi"));
    fhg::com::host_t host (fhg_kernel()->get("host", "*"));
    fhg::com::port_t port (fhg_kernel()->get("port", "0"));
    {
      const std::string master_names (fhg_kernel()->get("master", ""));
      const std::string virtual_capabilities (fhg_kernel()->get("capabilities", ""));
      fhg::util::split (master_names, ",", std::back_inserter(master_list));
      fhg::util::split (virtual_capabilities, ",", std::back_inserter(capability_list));
    }
    const std::size_t netd_nthreads (fhg_kernel()->get ("netd_nthreads", 4L));
    const std::string netd_url (fhg_kernel()->get ("netd_url", "tcp://*"));
    kvs::KeyValueStore* kvs (fhg_kernel()->acquire<kvs::KeyValueStore> ("kvs"));

    gspc::net::initialize (netd_nthreads);

    gspc::net::server::default_service_demux().handle
      ("/service/echo", gspc::net::service::echo ());

    m_server = gspc::net::serve
      (netd_url, gspc::net::server::default_queue_manager());

    kvs->put ("gspc.net.url." + name, m_server->url());


    m_shutting_down = false;

    m_reconnect_counter = 0;
    m_my_name = name;
    m_backlog_size = backlog_size;
    m_terminate_on_failure = terminate_on_failure;
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
          (std::make_pair ( cap
                          , std::make_pair ( sdpa::Capability ( cap_name
                                                              , cap_type
                                                              , m_my_name
                                                              )
                                           , new fhg::plugin::Capability( cap_name
                                                                        , cap_type
                                                                        )
                                           )
                          )
          );
      }
    }

    // TODO: add
    //
    //      acquire_all<T>() -> [(name, T*)]
    //
    // to get access to all plugins of a particular type
    if (cap)
    {
      MLOG( INFO, "gained capability: " << cap->capability_name()
          << " of type " << cap->capability_type()
          );

      boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);
      m_capabilities.insert
        (std::make_pair ( cap->capability_name()
                        , std::make_pair (sdpa::Capability ( cap->capability_name ()
                                                           , cap->capability_type ()
                                                           )
                                         , cap
                                         )
                        )
        );
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

    restore_jobs ();

    m_execution_thread.reset
      (new boost::thread(&DRTSImpl::job_execution_thread, this));
    fhg::util::set_threadname (*m_execution_thread, "[drts-execute]");

    start_connect ();

    gspc::net::server::default_service_demux().handle
      ("/service/drts/capability/add"
      , boost::bind (&DRTSImpl::service_capability_add, this, _1, _2, _3)
      );

    gspc::net::server::default_service_demux().handle
      ("/service/drts/capability/del"
      , boost::bind (&DRTSImpl::service_capability_del, this, _1, _2, _3)
      );

    gspc::net::server::default_service_demux().handle
      ("/service/drts/capability/get"
      , boost::bind (&DRTSImpl::service_capability_get, this, _1, _2, _3)
      );

    FHG_PLUGIN_STARTED();
  }
  catch (std::exception const &ex)
  {
    MLOG (ERROR, ex.what());
    FHG_PLUGIN_FAILED (-1);
  }


  FHG_PLUGIN_STOP()
  {
    gspc::net::server::default_service_demux().unhandle ("/service/drts/capability/add");
    gspc::net::server::default_service_demux().unhandle ("/service/drts/capability/del");
    gspc::net::server::default_service_demux().unhandle ("/service/drts/capability/get");

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

    {
      boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
      fhg_kernel()->storage()->save("jobs", m_jobs);
    }

    {
      while (not m_virtual_capabilities.empty())
      {
        delete m_virtual_capabilities.begin()->second.second;
        m_virtual_capabilities.erase(m_virtual_capabilities.begin());
      }
    }

    if (m_server)
    {
      m_server->stop ();
    }

    gspc::net::shutdown ();


    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    fhg::plugin::Capability *cap
      = fhg_kernel()->acquire<fhg::plugin::Capability>(plugin);
    if (cap)
    {
      MLOG( INFO
          , "gained capability: " << cap->capability_name()
          << " of type " << cap->capability_type()
          );

      boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);
      m_capabilities.insert
        (std::make_pair ( cap->capability_name()
                        , std::make_pair (sdpa::Capability ( cap->capability_name ()
                                                           , cap->capability_type ()
                                                           )
                                         , cap
                                         )
                        )
        );

      for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        if (master_it->second)
        {
          send_event
            (new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                      , master_it->first
                                                      , m_capabilities[cap->capability_name ()].first
                                                      )
            );
        }
      }
    }
  }

  FHG_ON_PLUGIN_UNLOAD()
  {
  }

  FHG_ON_PLUGIN_PREUNLOAD(plugin)
  {
    boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);
    map_of_capabilities_t::iterator cap(m_capabilities.find(plugin));
    if (cap != m_capabilities.end())
    {
      MLOG(INFO, "lost capability: " << plugin);
      MLOG(WARN, "TODO: make sure none of jobs make use of this capability");

      notify_capability_lost (cap->second.first);

      m_capabilities.erase(cap);
    }
  }

  // event handler callbacks
  //    implemented events
  virtual void handleWorkerRegistrationAckEvent
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
  virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e)
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

  virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*)
  {
  }

  virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*)
  {
  }

  virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent *)
  {
  }

  virtual void handleErrorEvent(const sdpa::events::ErrorEvent *)
  {
  }

  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent *)
  {
  }

  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent *)
  {
  }

  virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent *e)
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

        fhg_kernel()->storage()->save("jobs", m_jobs);

        m_pending_jobs.put(job);
      }
    }

    //    boost::mutex::scoped_lock lock(m_job_arrived_mutex);
    m_job_arrived.notify_all();
  }

  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent *e)
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

  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent *e)
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

    fhg_kernel()->storage()->save("jobs", m_jobs);
  }

  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *e)
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

    fhg_kernel()->storage()->save("jobs", m_jobs);
  }

  // not implemented events
  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *){}
  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent *) {}
  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *) {}
  virtual void handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *) {}
  virtual void handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *) {}
  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent *) {}
private:
  // threads
  void event_thread ()
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

  void job_execution_thread ()
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

        if (m_terminate_on_failure && job->state() == drts::Job::FAILED)
        {
          MLOG( WARN, "execution of job failed"
              << " and terminate on failure policy is in place."
              << " Good bye cruel world."
              );
          fhg_kernel()->terminate();
        }

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

          fhg_kernel()->storage()->save("jobs", m_jobs);
        }
      }
    }
  }

  void add_virtual_capability (std::string const &cap)
  {
    boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

    if (m_virtual_capabilities.find(cap) == m_virtual_capabilities.end())
    {
      DMLOG(TRACE, "adding virtual capability: " << cap);
      m_virtual_capabilities.insert
        (std::make_pair ( cap
                        , std::make_pair ( sdpa::Capability (cap
                                                            , "virtual"
                                                            , m_my_name
                                                            )
                                         , new fhg::plugin::Capability( cap
                                                                      , "virtual"
                                                                      )
                                         )
                        )
        );

      notify_capability_gained (m_virtual_capabilities[cap].first);
    }
  }

  void del_virtual_capability (std::string const &cap)
  {
    boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

    typedef map_of_capabilities_t::iterator cap_it_t;
    cap_it_t cap_it = m_virtual_capabilities.find (cap);
    if (cap_it != m_virtual_capabilities.end ())
    {
      notify_capability_lost (cap_it->second.first);

      delete cap_it->second.second;
      m_virtual_capabilities.erase (cap);
    }
  }

  void service_capability_add ( std::string const &
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

  void service_capability_del ( std::string const &
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

  void service_capability_get ( std::string const &
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              )
  {
    boost::mutex::scoped_lock cap_lock(m_capabilities_mutex);

    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    typedef map_of_capabilities_t::const_iterator const_cap_it_t;
    for ( const_cap_it_t cap_it (m_capabilities.begin())
        ; cap_it != m_capabilities.end()
        ; ++cap_it
        )
    {
      rply.add_body (cap_it->first + "\n");
    }

    for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
        ; cap_it != m_virtual_capabilities.end()
        ; ++cap_it
        )
    {
      rply.add_body (cap_it->first + "\n");
    }

    user->deliver (rply);
  }

  void notify_capabilities_to_master (std::string const &master)
  {
    sdpa::capabilities_set_t caps;
    boost::mutex::scoped_lock capabilities_lock(m_capabilities_mutex);

    typedef map_of_capabilities_t::const_iterator const_cap_it_t;
    for ( const_cap_it_t cap_it(m_capabilities.begin())
        ; cap_it != m_capabilities.end()
        ; ++cap_it
        )
    {
      caps.insert (cap_it->second.first);
    }

    for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
        ; cap_it != m_virtual_capabilities.end()
        ; ++cap_it
        )
    {
      caps.insert (cap_it->second.first);
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

  void notify_capability_gained (sdpa::Capability const &cap)
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

  void notify_capability_lost (sdpa::Capability const &cap)
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

  void restore_jobs()
  {
    map_of_jobs_t old_jobs;
    fhg_kernel()->storage()->load("jobs", old_jobs);
    for ( map_of_jobs_t::iterator it (old_jobs.begin()), end (old_jobs.end())
        ; it != end
        ; ++it
        )
    {
      boost::shared_ptr<drts::Job> job (it->second);

      switch (job->state())
      {
      case drts::Job::FINISHED:
        {
          boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
          MLOG(INFO, "restoring information of finished job: " << job->id());
          m_jobs[it->first] = job;
        }
        break;
      case drts::Job::FAILED:
        {
          boost::mutex::scoped_lock job_map_lock (m_job_map_mutex);
          MLOG(INFO, "restoring information of failed job: " << job->id());
          m_jobs[it->first] = job;
        }
        break;
      case drts::Job::PENDING:
        MLOG(WARN, "ignoring old pending job: " << job->id());
        break;
      case drts::Job::RUNNING:
        MLOG(WARN, "ignoring old running job: " << job->id());
        break;
      case drts::Job::CANCELED:
        MLOG(WARN, "ignoring old canceled job: " << job->id());
        break;
      default:
        MLOG(ERROR, "STRANGE job state: " << job->state());
        break;
      }
    }
  }

  void resend_outstanding_events (std::string const &master)
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

  void send_job_result_to_master (boost::shared_ptr<drts::Job> const & job)
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

  void start_connect ()
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
          fhg_kernel()->shutdown();
          return;
        }
      }
    }


    if (at_least_one_disconnected)
    {
      fhg_kernel()->schedule ( "connect"
                             , boost::bind( &DRTSImpl::start_connect
                                          , this
                                          )
                             , 5
                             );
    }
  }

  void start_receiver()
  {
    m_peer->async_recv(&m_message, boost::bind( &DRTSImpl::handle_recv
                                              , this
                                              , _1
                                              )
                      );
  }

  void handle_recv (boost::system::error_code const & ec)
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

          fhg_kernel()->schedule ( "connect"
                                 , boost::bind( &DRTSImpl::start_connect
                                              , this
                                              )
                                 , 5
                                 );
        }

        start_receiver();
      }
      else
      {
        MLOG(TRACE, m_peer->name() << " is shutting down");
      }
    }
  }

  void send_event (sdpa::events::SDPAEvent *e)
  {
    send_event(sdpa::events::SDPAEvent::Ptr(e));
  }

  void send_event (sdpa::events::SDPAEvent::Ptr const & evt)
  {
    static sdpa::events::Codec codec;

    const std::string encoded_evt (codec.encode(evt.get()));

    m_peer->send (evt->to(), encoded_evt);
  }

  void dispatch_event (sdpa::events::SDPAEvent::Ptr const &evt)
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

  bool m_shutting_down;
  bool m_terminate_on_failure;

  WFEImpl *m_wfe;

  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  std::string m_my_name;
  //! \todo Two sets for connected and unconnected masters?
  map_of_masters_t m_masters;
  std::size_t m_max_reconnect_attempts;
  std::size_t m_reconnect_counter;

  fhg::thread::queue<sdpa::events::SDPAEvent::Ptr>  m_event_queue;
  boost::shared_ptr<boost::thread>    m_event_thread;
  boost::shared_ptr<boost::thread>    m_execution_thread;

  mutable boost::mutex m_job_map_mutex;
  mutable boost::mutex m_job_computed_mutex;
  boost::condition_variable     m_job_computed;
  mutable boost::mutex m_job_arrived_mutex;
  mutable boost::mutex m_reconnect_counter_mutex;
  boost::condition_variable     m_job_arrived;

  fhg::util::thread::event<std::string> m_connected_event;

  mutable boost::mutex m_capabilities_mutex;
  map_of_capabilities_t m_capabilities;
  map_of_capabilities_t m_virtual_capabilities;

  // jobs + their states
  size_t m_backlog_size;
  map_of_jobs_t m_jobs;

  fhg::thread::queue<boost::shared_ptr<drts::Job> > m_pending_jobs;

  gspc::net::server_ptr_t m_server;
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "DRTS"
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
