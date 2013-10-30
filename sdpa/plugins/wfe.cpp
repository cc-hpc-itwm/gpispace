#include "wfe.hpp"
#include "observable.hpp"
#include "drts_info_impl.hpp"
#include <errno.h>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <list>
#include <map>
#include <string>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/bool.hpp>
#include <fhg/util/bool_io.hpp>
#include <fhg/util/threadname.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/join.hpp>
#include <fhg/error_codes.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>

#include <gspc/net.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.hpp>
//! \todo eliminate this include (that completes the type transition_t::data)
#include <we/type/net.hpp>

namespace
{
  struct wfe_task_t
  {
    typedef boost::posix_time::ptime time_type;
    typedef std::map<std::string, std::string> meta_data_t;
    typedef std::list<std::string> worker_list_t;

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
    we::mgmt::type::activity_t activity;
    fhg::util::thread::event<int> done;
    meta_data_t meta;
    worker_list_t workers;
    std::string error_message;
  };

  struct wfe_exec_context : public we::mgmt::context
  {
    wfe_exec_context (we::loader::loader& module_loader, wfe_task_t& target)
      : loader (module_loader)
      , task (target)
    {}

    virtual int handle_internally (we::mgmt::type::activity_t& act, net_t &)
    {
      act.inject_input();

      while (act.can_fire() && (task.state != wfe_task_t::CANCELED))
      {
        we::mgmt::type::activity_t sub (act.extract());
        sub.inject_input();
        sub.execute (this);
        act.inject (sub);
      }

      act.collect_output();

      return 0;
    }

    virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
    {
      try
      {
        module::call (loader, act, mod);
      }
      catch (std::exception const &ex)
      {
        throw std::runtime_error
          ( "call to '" + mod.module() + "::" + mod.function() + "'"
          + " failed: " + ex.what()
          );
      }

      return 0;
    }

    virtual int handle_internally (we::mgmt::type::activity_t&, expr_t&)
    {
      return 0;
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
    {
      return handle_internally (act, n);
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& module_call)
    {
      return handle_internally (act, module_call);
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
    {
      return handle_internally (act, e);
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

class WFEImpl : FHG_PLUGIN
              , public wfe::WFE
              , public observe::Observable
{
  typedef boost::mutex mutex_type;
  typedef boost::lock_guard<mutex_type> lock_type;

  typedef fhg::thread::queue< wfe_task_t *
                            , std::list
                            > task_list_t;
  typedef std::map<std::string, wfe_task_t *> map_of_tasks_t;
public:

  FHG_PLUGIN_START()
  {
    assert (! m_worker);
    m_loader = we::loader::loader::create();

    m_current_task = 0;
    m_auto_unload = fhg_kernel ()->get<fhg::util::bool_t> ("auto_unload", "false");

    {
      // initalize loader with paths
      std::string search_path
        (fhg_kernel()->get("library_path", fhg::util::getenv("PC_LIBRARY_PATH")));

      search_path_appender appender(*m_loader);
      fhg::util::split(search_path, ":", appender);

      if (search_path.empty())
      {
        MLOG(WARN, "loader has an empty search path, try setting environment variable PC_LIBRARY_PATH or variable plugin.wfe.library_path");
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

    m_worker.reset(new boost::thread(&WFEImpl::execution_thread, this));
    fhg::util::set_threadname (*m_worker, "[wfe]");

    gspc::net::handle
      ("/service/wfe/unload-modules"
      , boost::bind (&WFEImpl::service_module_unload, this, _1, _2, _3)
      );

    gspc::net::handle
      ("/service/wfe/current-job"
      , boost::bind (&WFEImpl::service_current_job, this, _1, _2, _3)
      );

    gspc::net::handle
      ("/service/wfe/search-path/set"
      , boost::bind (&WFEImpl::service_set_search_path, this, _1, _2, _3)
      );

    gspc::net::handle
      ("/service/wfe/search-path/get"
      , boost::bind (&WFEImpl::service_get_search_path, this, _1, _2, _3)
      );

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    gspc::net::unhandle ("/service/wfe/unload-modules");
    gspc::net::unhandle ("/service/wfe/current-job");
    gspc::net::unhandle ("/service/wfe/search-path/get");
    gspc::net::unhandle ("/service/wfe/search-path/set");

    if (m_worker)
    {
      m_worker->interrupt();
      boost::posix_time::time_duration timeout =
        boost::posix_time::seconds (15);
      if (not m_worker->timed_join (timeout))
      {
        LOG (WARN, "could not interrupt user-code, aborting");
        _exit (66);
      }
      else
      {
        m_worker.reset();
      }
    }

    {
      lock_type task_map_lock (m_mutex);
      while (! m_task_map.empty ())
      {
        wfe_task_t *task = m_task_map.begin ()->second;
        task->state = wfe_task_t::CANCELED;
        task->error_message = "plugin shutdown";
        task->done.notify(fhg::error::EXECUTION_CANCELLED);

        m_task_map.erase (task->id);
      }
    }

    if (m_loader)
    {
      m_loader.reset();
    }
    FHG_PLUGIN_STOPPED();
  }

  void emit_task ( const wfe_task_t& task
                 , sdpa::daemon::NotificationEvent::state_t state
                 )
  {
    emit ( sdpa::daemon::NotificationEvent
           (task.workers, task.id, state, task.activity, task.meta)
         );
  }


  int execute ( std::string const &job_id
              , std::string const &job_description
              , wfe::capabilities_t const&
              , std::string & result
              , std::string & error_message
              , std::list<std::string> const & worker_list
              , wfe::meta_data_t const & meta_data
              )
  {
    int ec = fhg::error::NO_ERROR;

    wfe_task_t task;
    task.state = wfe_task_t::PENDING;
    task.id = job_id;
    task.meta = meta_data;
    task.workers = worker_list;

    {
      lock_type task_map_lock(m_mutex);
      m_task_map.insert(std::make_pair(job_id, &task));
    }

    try
    {
      task.activity = we::mgmt::type::activity_t (job_description);

      // TODO get walltime from activity properties
      boost::posix_time::time_duration walltime = boost::posix_time::seconds(0);

      m_tasks.put(&task);

      if (walltime > boost::posix_time::seconds(0))
      {
        MLOG(INFO, "task has a walltime of " << walltime);
        if (! task.done.timed_wait(ec, boost::get_system_time()+walltime))
        {
          // abort task, i.e. restart worker

          // this is  required to ensure  that the execution thread  is actually
          // finished with this task
          task.done.wait(ec);

          task.state = wfe_task_t::FAILED;
          ec = fhg::error::WALLTIME_EXCEEDED;
        }
      }
      else
      {
        task.done.wait(ec);
      }

      result = task.activity.to_string();

      if (fhg::error::NO_ERROR == ec)
      {
        MLOG(TRACE, "task finished: " << task.id);
        task.state = wfe_task_t::FINISHED;
        error_message = task.error_message;

        emit_task (task, sdpa::daemon::NotificationEvent::STATE_FINISHED);
      }
      else if (fhg::error::EXECUTION_CANCELLED == ec)
      {
        DMLOG (TRACE, "task canceled: " << task.id << ": " << task.error_message);
        task.state = wfe_task_t::CANCELED;
        error_message = task.error_message;

        emit_task (task, sdpa::daemon::NotificationEvent::STATE_CANCELLED);
      }
      else
      {
        MLOG (ERROR, "task failed: " << task.id << ": " << task.error_message);
        task.state = wfe_task_t::FAILED;
        error_message = task.error_message;

        emit_task (task, sdpa::daemon::NotificationEvent::STATE_FAILED);
      }
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not parse activity: " << ex.what());
      task.state = wfe_task_t::FAILED;
      ec = fhg::error::INVALID_JOB_DESCRIPTION;
      error_message = ex.what();

      emit_task (task, sdpa::daemon::NotificationEvent::STATE_FAILED);
    }

    {
      lock_type task_map_lock(m_mutex);
      m_task_map.erase (job_id);
    }

    return ec;
  }

  int cancel (std::string const &job_id)
  {
    lock_type job_map_lock(m_mutex);
    map_of_tasks_t::iterator task_it (m_task_map.find(job_id));
    if (task_it == m_task_map.end())
    {
      MLOG(WARN, "could not find task " << job_id);
      return -ESRCH;
    }
    else
    {
      task_it->second->state = wfe_task_t::CANCELED;
    }

    return 0;
  }
private:
  void service_module_unload ( std::string const &dst
                             , gspc::net::frame const &rqst
                             , gspc::net::user_ptr user
                             )
  {
    MLOG (INFO, "unloading modules as requested by the user...");

    gspc::net::frame rply;

    try
    {
      m_loader->unload_autoloaded ();
      rply = gspc::net::make::reply_frame (rqst);
    }
    catch (std::exception const &ex)
    {
      rply = gspc::net::make::error_frame
        ( rqst
        , gspc::net::E_SERVICE_FAILED
        , ex.what ()
        );
    }

    user->deliver (rply);
  }

  void service_current_job ( std::string const &dst
                           , gspc::net::frame const &rqst
                           , gspc::net::user_ptr user
                           )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    {
      lock_type lock (m_current_task_mutex);
      if (m_current_task)
      {
        rply.set_body (m_current_task->activity.nice_name());
      }
    }

    user->deliver (rply);
  }

  void service_get_search_path ( std::string const &dst
                               , gspc::net::frame const &rqst
                               , gspc::net::user_ptr user
                               )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    {
      lock_type lock (m_mutex);

      rply.set_body
        (fhg::util::join ( m_loader->search_path ().begin ()
                         , m_loader->search_path ().end ()
                         , ":"
                         )
        );
    }

    user->deliver (rply);
  }

  void service_set_search_path ( std::string const &dst
                               , gspc::net::frame const &rqst
                               , gspc::net::user_ptr user
                               )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    {
      std::string search_path (rqst.get_body ());

      search_path_appender appender(*m_loader);
      lock_type lock (m_mutex);
      m_loader->clear_search_path ();
      fhg::util::split(search_path, ":", appender);
    }

    user->deliver (rply);
  }

  void execution_thread ()
  {
    for (;;)
    {
      {
        lock_type lock (m_current_task_mutex);
        m_current_task = 0;
      }

      wfe_task_t *task = m_tasks.get();

      {
        lock_type lock (m_current_task_mutex);
        m_current_task = task;
      }

      emit_task (*task, sdpa::daemon::NotificationEvent::STATE_STARTED);

      if (task->state != wfe_task_t::PENDING)
      {
        task->errc = fhg::error::EXECUTION_CANCELLED;
        task->error_message = "cancelled";
      }
      else
      {
        try
        {
          wfe_exec_context ctxt (*m_loader, *task);

          gspc::drts::info::set_worker_list (task->workers);

          task->activity.inject_input();
          task->activity.execute (&ctxt);
          task->activity.collect_output();

          if (task->state == wfe_task_t::CANCELED)
          {
            task->errc = fhg::error::EXECUTION_CANCELLED;
            task->error_message = "cancelled";
          }
          else
          {
            task->state = wfe_task_t::FINISHED;
            task->errc = fhg::error::NO_ERROR;
            task->error_message = "success";
          }
        }
        catch (std::exception const & ex)
        {
          task->state = wfe_task_t::FAILED;
          // TODO: more detailed error codes
          task->errc = fhg::error::MODULE_CALL_FAILED;
          task->error_message = ex.what();

          if (m_auto_unload)
            m_loader->unload_autoloaded();
        }
        catch (...)
        {
          task->state = wfe_task_t::FAILED;
          task->errc = fhg::error::UNEXPECTED_ERROR;
          task->error_message =
            "UNKNOWN REASON, exception not derived from std::exception";
          task->done.notify(1);

          if (m_auto_unload)
            m_loader->unload_autoloaded();
        }
      }

      task->done.notify(task->errc);
    }
  }

  mutable mutex_type m_mutex;
  map_of_tasks_t m_task_map;
  task_list_t m_tasks;

  mutable mutex_type m_current_task_mutex;
  wfe_task_t *m_current_task;

  we::loader::loader::ptr_t m_loader;
  boost::shared_ptr<boost::thread> m_worker;

  bool m_auto_unload;
};

EXPORT_FHG_PLUGIN( wfe
                 , WFEImpl
                 , "WFE"
                 , "provides access to a workflow-engine"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
