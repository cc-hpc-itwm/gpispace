#include "wfe.hpp"
#include "wfe_task.hpp"
#include "wfe_context.hpp"
#include "observable.hpp"
#include "task_event.hpp"

#include <errno.h>

#include <list>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
              , public observe::Observable
{
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  typedef fhg::thread::queue< wfe_task_t *
                            , std::list
                            > task_list_t;
  typedef std::map<std::string, wfe_task_t *> map_of_tasks_t;
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
        MLOG(WARN, "loader has an empty search path, try setting environment variable PC_LIBRARY_PATH or variable plugin.wfe.library_path");
      }
      else
      {
        MLOG(INFO, "initialized loader with search path: " << search_path);
      }

      // TODO: figure out, why this doesn't work as it is supposed to
      // adjust ld_library_path
      std::string ld_library_path (fhg::util::getenv("LD_LIBRARY_PATH", ""));
      ld_library_path = search_path + ":" + ld_library_path;
      setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), true);
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
              , wfe::meta_data_t const & meta_data
              )
  {
    int ec = -EINVAL;

    wfe_task_t task;
    task.state = wfe_task_t::PENDING;
    task.id = job_id;
    task.capabilities = capabilities;
    task.meta = meta_data;

    {
      lock_type task_map_lock(m_mutex);
      m_task_map.insert(std::make_pair(job_id, &task));
    }

    try
    {
      task.activity = we::util::text_codec::decode<we::activity_t>(job_description);

      // TODO get walltime from activity properties
      boost::posix_time::time_duration walltime = boost::posix_time::seconds(0);

      emit(task_event_t( job_id
                       , task.activity.transition().name()
                       , task_event_t::ENQUEUED
                       , job_description
                       , task.meta
             )
          );

      task.enqueue_time = boost::posix_time::microsec_clock::universal_time();

      m_tasks.put(&task);

      if (walltime > boost::posix_time::seconds(0))
      {
        MLOG(INFO, "task has a walltime of " << walltime);
        if (! task.done.timed_wait(ec, boost::get_system_time()+walltime))
        {
          task.state = wfe_task_t::FAILED;
          ec = ETIMEDOUT;
          // this is  required to ensure  that the execution thread  is actually
          // finished with this task
          task.done.wait(ec);
        }
      }
      else
      {
        task.done.wait(ec);
      }

      task.finished_time = boost::posix_time::microsec_clock::universal_time();

      if (0 == ec)
      {
        MLOG(TRACE, "task finished: " << task.id);
        task.state = wfe_task_t::FINISHED;
        result = task.result;

        emit(task_event_t( job_id
                         , task.activity.transition().name()
                         , task_event_t::FINISHED
                         , we::util::text_codec::encode(task.activity)
                         , task.meta
                         )
            );
      }
      else if (ec < 0)
      {
        MLOG(WARN, "task canceled: " << task.id << ": " << strerror(-ec) << ": " << task.result);
        task.state = wfe_task_t::CANCELED;
        result = task.result;

        emit(task_event_t( job_id
                         , task.activity.transition().name()
                         , task_event_t::CANCELED
                         , we::util::text_codec::encode(task.activity)
                         , task.meta
                         )
            );
      }
      else
      {
        MLOG(WARN, "task failed: " << task.id << ": " << task.result);
        task.state = wfe_task_t::FAILED;
        result = task.result;

        emit(task_event_t( job_id
                         , task.activity.transition().name()
                         , task_event_t::FAILED
                         , we::util::text_codec::encode(task.activity)
                         , task.meta
                         )
            );
      }
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not parse activity: " << ex.what());
      task.state = wfe_task_t::FAILED;
      ec = EINVAL;

      emit(task_event_t( job_id
                       , "n/a"
                       , task_event_t::UNKNOWN
                       , ex.what()
                       , task.meta
                       )
          );
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
  void execution_thread ()
  {
    for (;;)
    {
      wfe_task_t *task = m_tasks.get();
      task->dequeue_time = boost::posix_time::microsec_clock::universal_time();

      emit(task_event_t( task->id
                       , task->activity.transition().name()
                       , task_event_t::DEQUEUED
                       , we::util::text_codec::encode(task->activity)
                       , task->meta
                       )
          );

      if (task->state != wfe_task_t::PENDING)
      {
        task->done.notify(-ECANCELED);
      }
      else
      {
        try
        {
          wfe_exec_context ctxt (*m_loader, *task);

          task->activity.inject_input();
          task->activity.execute (ctxt);
          task->activity.collect_output();

          if (task->state == wfe_task_t::CANCELED)
          {
            task->result = we::util::text_codec::encode(task->activity);
            task->done.notify(-ECANCELED);
          }
          else
          {
            task->state = wfe_task_t::FINISHED;
            task->result = we::util::text_codec::encode(task->activity);
            task->done.notify(0);
          }
        }
        catch (std::exception const & ex)
        {
          task->state = wfe_task_t::FAILED;
          task->result = ex.what();
          task->done.notify(1);

          m_loader->unload_autoloaded();
        }
      }
    }
  }

  mutable mutex_type m_mutex;
  map_of_tasks_t m_task_map;
  task_list_t m_tasks;
  we::loader::loader::ptr_t m_loader;
  boost::shared_ptr<boost::thread> m_worker;
};

EXPORT_FHG_PLUGIN( wfe
                 , WFEImpl
                 , "provides access to a workflow-engine"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
