#include <gspc/Worker.hpp>

#include <gspc/comm/worker/scheduler/Client.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/finish_reason/Cancelled.hpp>
#include <gspc/job/finish_reason/Finished.hpp>
#include <gspc/task/Result.hpp>

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
      auto const& inputs (task.inputs);

      auto value_at
        ( [&] (auto key)
          {
            return inputs.equal_range (key).first->second;
          }
        );

      if (task.so == "map_so" && task.symbol == "identity")
      {
        if (  inputs.size() != 3
           || inputs.count ("input") != 1
           || inputs.count ("output") != 1
           || inputs.count ("N") != 1
           || value_at ("input") != task.id.id
           || value_at ("input") + value_at ("output") != value_at ("N")
           )
        {
          throw std::logic_error ("Worker::execute: Map: Corrupted task.");
        }

        return {inputs};
      }

      if (task.so == "graph_so" && task.symbol == "static_map")
      {
        // do not generate children
        if (  inputs.size() != 1
           || inputs.count ("parent") != 1
           )
        {
          throw std::logic_error
            ("Worker::execute: Graph: Static Map: Corrupted task.");
        }

        return {inputs};
      }

      if (task.so == "graph_so" && task.symbol == "dynamic_map")
      {
        // when parent == 0 then generate all tasks [1..N]
        // when parent != 0 then verify its less or equal to N
        if (  inputs.size() != 2
           || inputs.count ("parent") != 1
           || inputs.count ("N") != 1
           || value_at ("parent") > value_at ("N")
           )
        {
          throw std::logic_error
            ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
        }

        auto outputs (inputs);

        auto const parent (value_at ("parent"));

        if (parent == 0)
        {
          auto n (value_at ("N"));

          while (n --> 0)
          {
            outputs.emplace ("children", n + 1);
          }
        }

        return {outputs};
      }

      if (task.so == "graph_so" && task.symbol == "nary_tree")
      {
        if (  inputs.count ("parent") != 1
           || inputs.count ("N") != 1
           || inputs.count ("B") != 1
           || !(value_at ("parent") < value_at ("N"))
           )
        {
          throw std::logic_error
            ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
        }

        auto outputs (inputs);

        auto b (value_at ("B"));
        auto const n (value_at ("N"));
        auto const parent (value_at ("parent"));

        outputs.emplace
          ( "heureka"
          , inputs.count ("heureka_value")
          && parent == value_at ("heureka_value")
          );

        auto const child_base (b * parent + 1);

        while (b --> 0)
        {
          auto const child (child_base + b);

          if (child < n)
          {
            outputs.emplace ("children", child);
          }
        }

        return {outputs};
      }

      throw std::invalid_argument
        ("Worker:execute_task: non-Map, non-Graph: NYI.");
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

  void Worker::cancel (job::ID id)
  {
    if (auto const work_item = _work_queue.remove (id))
    {
      //! \todo steal work == cancel_only_when_not_yet_started
      auto const& scheduler (work_item->first.scheduler);
      auto const& job_id (work_item->second);
      assert (job_id == id);

      comm::worker::scheduler::Client (_io_service_for_scheduler, scheduler)
        .finished (job_id, job::finish_reason::Cancelled{});
    }
    else
    {
      //! \todo
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
