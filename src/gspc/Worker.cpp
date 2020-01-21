#include <gspc/Worker.hpp>

#include <gspc/comm/worker/scheduler/Client.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/finish_reason/Cancelled.hpp>
#include <gspc/job/finish_reason/Finished.hpp>
#include <gspc/module_api.hpp>
#include <gspc/task/Result.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <exception>
#include <iterator>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace gspc
{
  Worker::Worker (Resource resource)
    : _resource {std::move (resource)}
    , _comm_server_for_scheduler (this)
    , _worker_thread (fhg::util::bind_this (this, &Worker::work))
  {}

  Worker::~Worker()
  {
    _work_queue.interrupt();

    if (_worker_thread.joinable())
    {
      _worker_thread.join();
    }
  }

  namespace
  {
    task::result::Success execute_task (Task const& task)
    {
      fhg::util::scoped_dlhandle const dl {task.so};
      auto const functions
        (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl, gspc_module_functions));
      return {functions->at (task.symbol) (task.inputs)};
    }

    job::finish_reason::Finished execute_job (Job const& job)
    {
      return {[&] { return execute_task (job.task); }};
    }
  }

  void Worker::work()
  try
  {
    while (true)
    {
      auto const work_item (_work_queue.pop());
      auto const& scheduler (work_item.first.scheduler);
      auto const& job_id (work_item.second);
      auto const& job (work_item.first.job);

      comm::worker::scheduler::Client (_io_service_for_scheduler, scheduler)
        .finished (job_id, execute_job (job));
    }
  }
  catch (WorkQueue::Interrupted const&)
  {
    // do nothing
  }
  //! \todo terminates when Client ctor throws

  rpc::endpoint Worker::endpoint_for_scheduler() const
  {
    return _comm_server_for_scheduler.local_endpoint();
  }

  void Worker::submit (rpc::endpoint scheduler, job::ID job_id, Job job)
  {
    _work_queue.push
      (WorkItem {std::move (scheduler), std::move (job)}, std::move (job_id));
  }

  void Worker::cancel (job::ID id, task::result::Premature reason)
  {
    if (auto const work_item = _work_queue.remove (id))
    {
      //! \todo steal work == cancel_only_when_not_yet_started
      auto const& scheduler (work_item->first.scheduler);
      auto const& job_id (work_item->second);
      assert (job_id == id);

      comm::worker::scheduler::Client (_io_service_for_scheduler, scheduler)
        .finished (job_id, job::finish_reason::Cancelled {reason});
    }
    else
    {
      //! \todo
      //! - remember reason
      //! - interrupt execution
      //! - if task succeeds after getting cancel: success or cancelled?
      //!   - vote: success
      //! - if task fails after getting cancel: fail or cancelled?
      //!   - list of reasons?
      //! - sanity: job was never known?
      //! - race: finished before we got the cancel -> DON'T throw if
      //!   not in queue or actively executing.
    }
  }
}
