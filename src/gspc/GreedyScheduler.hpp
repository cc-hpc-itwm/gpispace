#pragma once

#include <gspc/ScopedRuntimeSystem.hpp>
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
    struct Extract{};
    struct CancelAllTasks{};
    struct Finished
    {
      job::ID job_id;
      task::Result task_result;
    };
    struct Inject
    {
      task::ID task_id;
      task::Result task_result;
    };
    struct Cancelled
    {
      job::ID job_id;
    };

    using Command = boost::variant < Submit
                                   , Extract
                                   , CancelAllTasks
                                   , Finished
                                   , Inject
                                   , Cancelled
                                   >;

    using CommandQueue = fhg::util::threadsafe_queue<Command>;
    using ScheduleQueue =
      gspc::threadsafe_interruptible_queue_with_remove<Task, task::ID>;

    ScheduleQueue _schedule_queue;
    CommandQueue _command_queue;

    std::atomic<bool> _stopped {false};

    //! state only modified by command_thread
    std::unordered_map<job::ID, resource::ID> _jobs;
    std::unordered_map<task::ID, job::ID> _job_by_task;
    std::uint64_t _next_job_id {0};

    std::thread _schedule_thread;
    void schedule_thread();

    std::thread _command_thread;
    void command_thread();

    template<typename Function>
      void do_worker_call (resource::ID, Function&& function);
  };
}
