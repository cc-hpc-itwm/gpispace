#include "wfe.hpp"
#include "wfe_task.hpp"
#include "wfe_context.hpp"

#include <errno.h>

#include <list>

#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/getenv.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>

#include <we/we.hpp>

struct search_path_appender
{
  explicit
  search_path_appender(we::loader::loader& ld)
    : loader (ld)
  {}

  search_path_appender & operator = (std::string const &p)
  {
    loader.append_search_path (p);
    return *this;
  }

  search_path_appender & operator* ()
  {
    return *this;
  }

  search_path_appender & operator++(int)
  {
    return *this;
  }

  we::loader::loader & loader;
};

class WFEImpl : FHG_PLUGIN
              , public wfe::WFE
{
  typedef fhg::thread::queue< wfe_task_t *
                            , std::list
                            > task_list_t;

public:
  virtual ~WFEImpl() {}

  FHG_PLUGIN_START()
  {
    assert (! m_worker);
    m_loader = we::loader::loader::create();

    {
      // initalize loader with paths
      std::string search_path
        (fhg_kernel()->get("library_path", fhg::util::getenv("PC_LIBRARY_PATH")));

      search_path_appender appender(*m_loader);
      fhg::util::split(search_path, ":", appender);

      if (search_path.empty())
      {
        LOG(WARN, "loader has an empty search path, try setting environment variable PC_LIBRARY_PATH or variable plugin.wfe.library_path");
      }
      else
      {
        LOG(INFO, "initialized loader with search path: " << search_path);
      }
    }

    m_worker.reset(new boost::thread(&WFEImpl::execution_thread, this));
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    if (m_worker)
    {
      m_worker->interrupt();
      m_worker->join();
      m_worker.reset();
    }

    if (m_loader)
    {
      m_loader.reset();
    }
    FHG_PLUGIN_STOPPED();
  }

  int execute ( std::string const &job_id
              , std::string const &job_description
              , wfe::capabilities_t const & capabilities
              , std::string & result
              , boost::posix_time::time_duration const & walltime
              )
  {
    int ec = -EINVAL;

    MLOG(INFO, "executing...");

    try
    {
      wfe_task_t task;
      task.state = wfe_task_t::PENDING;
      task.id = job_id;
      task.activity = we::util::text_codec::decode<we::activity_t>(job_description);
      task.capabilities = capabilities;

      m_tasks.put(&task);

      if (walltime > boost::posix_time::seconds(0))
      {
        LOG(INFO, "task has a walltime of " << walltime);
        if (! task.done.timed_wait(ec, boost::get_system_time()+walltime))
        {
          task.state = wfe_task_t::CANCELED;
          ec = -ETIMEDOUT;
        }
      }
      else
      {
        task.done.wait(ec);
      }

      if (0 == ec)
      {
        MLOG(INFO, "task finished: " << task.id);
        result = task.result;
      }
      else
      {
        MLOG(WARN, "task failed: " << task.id << ": " << strerror(-ec) << ": " << task.result);
        result = task.result; //we::util::text_codec::encode(task.activity);
      }
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not parse activity: " << ex.what());
      return -EINVAL;
    }

    return ec;
  }

  int cancel (std::string const &job_id)
  {
    return -ESRCH;
  }
private:
  void execution_thread ()
  {
    for (;;)
    {
      wfe_task_t *task = m_tasks.get();
      if (task->state == wfe_task_t::CANCELED)
      {
        task->done.notify(-ECANCELED);
      }
      else
      {
        wfe_exec_context ctxt (*m_loader, *task);
        try
        {
          task->activity.execute (ctxt);
          task->result = we::util::text_codec::encode(task->activity);
          task->done.notify(0);
        }
        catch (std::exception const & ex)
        {
          task->result = ex.what();
          task->done.notify(-EFAULT);
        }
      }
    }
  }

  we::loader::loader::ptr_t m_loader;
  task_list_t m_tasks;
  boost::shared_ptr<boost::thread> m_worker;
};

EXPORT_FHG_PLUGIN( wfe
                 , WFEImpl
                 , "provides access to a workflow-engine"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
