#pragma once

#include <gspc/Job.hpp>
#include <gspc/Resource.hpp>
#include <gspc/Task.hpp>
#include <gspc/comm/scheduler/worker/Server.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/job/finish_reason/Finished.hpp>
#include <gspc/rpc/TODO.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/threadsafe_interruptible_queue_with_remove.hpp>

#include <util-generic/dynamic_linking.hpp>
#include <util-generic/hash/boost/filesystem/path.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>
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
    void cancel (job::ID, task::result::Premature);

    // called by user via scheduler (via rif)
    // TaskState status (job::ID);

  private:
    Resource _resource;

    fhg::util::scoped_boost_asio_io_service_with_threads
      _io_service_for_scheduler {1};
    comm::scheduler::worker::Server const _comm_server_for_scheduler;

    struct WorkItem
    {
      rpc::endpoint scheduler;
      Job job;
    };

    using WorkQueue =
      threadsafe_interruptible_queue_with_remove<WorkItem, job::ID>;

    WorkQueue _work_queue;
    std::thread _worker_thread;
    void work();

    task::result::Success execute_task (Task const&);
    job::finish_reason::Finished execute_job (Job const&);

    std::unordered_map< boost::filesystem::path
                      , std::unique_ptr<fhg::util::scoped_dlhandle>
                      > _so_handles;
  };
}
