#pragma once

#include <gspc/Job.hpp>
#include <gspc/Resource.hpp>
#include <gspc/comm/scheduler/worker/Server.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/rpc/TODO.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <condition_variable>
#include <forward_list>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace gspc
{
  class Worker
  {
  public:
    Worker (Resource);

    Worker (Worker const&) = delete;
    Worker (Worker&&) = delete;
    Worker& operator= (Worker const&) = delete;
    Worker& operator= (Worker&&) = delete;
    //! \note dtor waits? cancels?
    ~Worker();

    rpc::endpoint endpoint_for_scheduler() const;

    // called by scheduler (via rif)
    //! \note WorkQueue implies fifo for submitted jobs.
    void submit (rpc::endpoint, job::ID, Job);
    void cancel (job::ID);

    // called by user via scheduler (via rif)
    // TaskState status (job::ID);

  private:
    Resource _resource;

    fhg::util::scoped_boost_asio_io_service_with_threads
      _io_service_for_scheduler {1};
    comm::scheduler::worker::Server const _comm_server_for_scheduler;

    //! \todo name: Job? Task?
    struct WorkItem
    {
      rpc::endpoint scheduler;
      job::ID job_id;
      Job job;
    };

    struct WorkQueue
    {
      // duplicate element: throw, queue unchanged
      void push (WorkItem);

      // empty: throw, queue unchanged
      // return: oldest element (fifo)
      WorkItem pop();

      // unknown element: none, queue unchanged
      boost::optional<WorkItem> remove (job::ID);

      struct Interrupted {};
      void interrupt();

    private:
      std::mutex _guard;
      bool _interrupted = false;
      std::condition_variable _work_items_added_or_interrupted;

      using Elements = std::forward_list<WorkItem>;
      using Position = typename Elements::const_iterator;
      using Positions = std::unordered_map<job::ID, Position>;

      Elements _work_items;
      Positions _positions;
      Position _before {_work_items.cbefore_begin()};

      //! Requires lock being held.
      boost::optional<WorkItem> do_remove (job::ID);
    };

    WorkQueue _work_queue;
    std::thread _worker_thread;
    void work();
  };
}
