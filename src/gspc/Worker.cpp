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
    task::Result execute_task (Task const& task)
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
        if (  inputs.size() != 3
           || inputs.count ("parent") != 1
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
        auto const child_base (b * value_at ("parent") + 1);

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
      auto const& scheduler (work_item.scheduler);
      auto const& job_id (work_item.job_id);
      auto const& job (work_item.job);

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
      (WorkItem {std::move (scheduler), std::move (job_id), std::move (job)});
  }

  void Worker::cancel (job::ID id)
  {
    if (auto const work_item = _work_queue.remove (id))
    {
      auto const& scheduler (work_item->scheduler);
      auto const& job_id (work_item->job_id);
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

  void Worker::WorkQueue::interrupt()
  {
    std::lock_guard<std::mutex> const lock (_guard);
    _interrupted = true;
    _work_items_added_or_interrupted.notify_all();
  }

  void Worker::WorkQueue::push (Worker::WorkItem item)
  try
  {
    std::lock_guard<std::mutex> const lock (_guard);

    if (!_positions.emplace (item.job_id, _before).second)
    {
      throw std::invalid_argument ("Duplicate.");
    }

    _before = _work_items.insert_after (_before, item);

    _work_items_added_or_interrupted.notify_one();
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        ( ( boost::format ("WorkQueue::push (job %1%)")
          % item.job_id
          ).str()
        )
      );
  }

  Worker::WorkItem Worker::WorkQueue::pop()
  try
  {
    std::unique_lock<std::mutex> lock (_guard);
    _work_items_added_or_interrupted.wait
      (lock, [this] { return !_work_items.empty() || _interrupted; });

    if (_interrupted)
    {
      throw Interrupted();
    }

    return *do_remove (_work_items.front().job_id);
  }
  catch (Interrupted const&)
  {
    throw;
  }
  catch (...)
  {
    std::throw_with_nested (std::runtime_error ("WorkQueue::pop()"));
  }

  boost::optional<Worker::WorkItem> Worker::WorkQueue::remove (job::ID id)
  try
  {
    std::lock_guard<std::mutex> const lock (_guard);

    return do_remove (id);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          ( ( boost::format ("WorkQueue::remove (id %1%)")
            % id
            ).str()
          )
      );
  }

  boost::optional<Worker::WorkItem> Worker::WorkQueue::do_remove (job::ID id)
  {
    auto position {_positions.find (id)};

    if (position == _positions.end())
    {
      return boost::none;
    }

    auto before {position->second};
    auto item (std::move (*std::next (before)));
    auto next {std::next (before, 2)};

    if (next != _work_items.end())
    {
      _positions[next->job_id] = before;
    }
    else
    {
      _before = before;
    }

    _positions.erase (position);
    _work_items.erase_after (before);

    return item;
  }
}
