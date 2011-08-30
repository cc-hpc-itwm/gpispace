#include "wfe.hpp"

#include <errno.h>

#include <list>

#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>

#include <we/we.hpp>

struct wfe_task_t
{
  enum state_t
    {
      PENDING
    , CANCELED
    , FINISHED
    };

  std::string id;
  int        state;
  we::activity_t activity;
  fhg::util::thread::event<int> done;
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

    FHG_PLUGIN_STOPPED();
  }

  int execute ( std::string const &job_id
              , std::string const &job_description
              , wfe::capabilities_t const & capabilities
              , std::string & result
              , boost::posix_time::time_duration = boost::posix_time::seconds(0)
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

      m_tasks.put(&task);

      task.done.wait(ec);

      if (0 == ec)
      {
        MLOG(INFO, "task finished");
        result = we::util::text_codec::encode(task.activity);
      }
      else
      {
        LOG(WARN, "task failed: " << task.id << ": " << strerror(-ec));
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
      if (task->CANCELED)
      {
        task->done.notify(-ECANCELED);
      }
      else
      {
        task->done.notify(-ECANCELED);
      }
    }
  }

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
