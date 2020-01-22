#pragma once

#include <gspc/BoundedStorageEvictMiddle.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/comm/scheduler/worker/Client.hpp>
#include <gspc/comm/scheduler/workflow_engine/Client.hpp>
#include <gspc/interface/Scheduler.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/resource_manager/Trivial.hpp>
#include <gspc/threadsafe_interruptible_queue_with_remove.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace gspc
{
  class GreedyScheduler : public interface::Scheduler
  {
  public:
    //! \note: workflow_engine can not be shared with other schedulers
    //! because alien inject would break the finish condition

    //! \todo also applies to put_token: either managed by scheduler
    //! or requires notification from workflow_engine to
    //! scheduler. Possible change in API: let
    //! workflow_engine::extract() block
    GreedyScheduler ( comm::scheduler::workflow_engine::Client
                    , resource_manager::Trivial& //! \todo: Client
                    , ScopedRuntimeSystem& //! \todo UnscopedBase
                    );
    ~GreedyScheduler();

    virtual void wait() override;
    virtual void stop() override;
    virtual void finished (job::ID, job::FinishReason) override;

  private:
    comm::scheduler::workflow_engine::Client _workflow_engine;
    resource_manager::Trivial& _resource_manager;
    ScopedRuntimeSystem& _runtime_system;

    struct Submit
    {
      Task task;
      resource_manager::Trivial::Acquired acquired;
    };
    struct FailedToAcquire
    {
      Task task;
      std::exception_ptr error;
    };
    struct Extract{};
    struct Stop
    {
      std::string reason;
    };
    struct Finished
    {
      job::ID job_id;
      task::Result task_result;
    };
    struct Cancelled
    {
      job::ID job_id;
      task::result::Premature reason;
    };

    using Command = boost::variant < Submit
                                   , FailedToAcquire
                                   , Extract
                                   , Stop
                                   , Finished
                                   , Cancelled
                                   >;

    using CommandQueue = fhg::util::threadsafe_queue<Command>;
    using ScheduleQueue =
      gspc::threadsafe_interruptible_queue_with_remove<Task, task::ID>;

    ScheduleQueue _schedule_queue;
    CommandQueue _command_queue;

    //! state only modified by command_thread
    std::unordered_map<job::ID, resource::ID> _jobs;
    std::unordered_map<task::ID, job::ID> _job_by_task;
    std::uint64_t _next_job_id {0};

    bool _stopping {false};

    // manually maintained _schedule_queue.size() + in flight in schedule thread
    std::size_t _scheduling_items {0};
    void schedule_queue_push (Task);
    bool schedule_queue_remove (task::ID);

    std::thread _schedule_thread;
    void schedule_thread();

    std::thread _command_thread;
    void command_thread();
    void remove_job (job::ID);
    void cancel_job (job::ID, task::result::Premature);
    void cancel_task (task::ID, task::result::Premature);
    void inject (task::ID, task::Result);

    //! \todo parameter for cache size
    BoundedStorageEvictMiddle<resource::ID, comm::scheduler::worker::Client>
      _worker_clients {100};

    template<typename Function>
      void do_worker_call (resource::ID, Function&& function);
    void stop_with_reason (std::string);
  };
}
