#include <gspc/Worker.hpp>

#include <gspc/comm/worker/scheduler/Client.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/finish_reason/Finished.hpp>
#include <gspc/task/Result.hpp>

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
    task::Result execute_task (Task const& task)
    {
      auto const& inputs (task.inputs);

      if (task.so == "map_so" && task.symbol == "identity")
      {
        if (  inputs.size() != 3
           || inputs.count ("input") != 1
           || inputs.count ("output") != 1
           || inputs.count ("N") != 1
           || inputs.at ("input") != task.id.id
           || inputs.at ("input") + inputs.at ("output") != inputs.at ("N")
           )
        {
          throw std::logic_error ("Worker::execute: Map: Corrupted task.");
        }

        return {inputs};
      }

      throw std::invalid_argument ("Worker:execute_task: non-Map: NYI.");
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
      auto const work_item (_work_queue.get());
      auto const& scheduler (work_item.scheduler);
      auto const& job_id (work_item.job_id);
      auto const& job (work_item.job);

      comm::worker::scheduler::Client (_io_service_for_scheduler, scheduler)
        .finished (job_id, execute_job (job));
    }
  }
  catch (WorkQueue::interrupted const&)
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
    _work_queue.put
      (WorkItem {std::move (scheduler), std::move (job_id), std::move (job)});
  }
  void Worker::cancel (job::ID)
  {
    throw std::runtime_error ("Worker::cancel: NYI");
  }
}
