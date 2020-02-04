#pragma once

#include <gspc/BoundedStorageEvictMiddle.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/comm/scheduler/worker/Client.hpp>
#include <gspc/comm/scheduler/workflow_engine/Client.hpp>
#include <gspc/interface/Scheduler.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/resource_manager/WithPreferences.hpp>
#include <gspc/threadsafe_interruptible_queue_with_remove.hpp>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
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
                    , resource_manager::WithPreferences& //! \todo: Client
                    , ScopedRuntimeSystem& //! \todo UnscopedBase

                    , std::size_t max_attempt = 1
                    );
    ~GreedyScheduler();

    virtual void wait() override;
    virtual void stop() override;
    virtual void finished (job::ID, job::FinishReason) override;

  private:
    comm::scheduler::workflow_engine::Client _workflow_engine;
    resource_manager::WithPreferences& _resource_manager;
    ScopedRuntimeSystem& _runtime_system;

    std::size_t _max_attempts;

    struct Submit
    {
      task::ID task_id;
      resource_manager::WithPreferences::Acquired acquired;
      task::Implementation task_implementation;
    };
    struct FailedToAcquire
    {
      task::ID task_id;
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

    struct CommandQueue
    {
      Command get();

      void put_submit ( task::ID
                      , resource_manager::WithPreferences::Acquired
                      , task::Implementation
                      );
      void put_failed_to_acquire (task::ID, std::exception_ptr);
      void put_extract();
      void put_stop (std::string);
      void put_finished (job::ID, task::Result);
      void put_cancelled (job::ID, task::result::Premature);

    private:
      std::mutex _guard;
      std::condition_variable _command_added;

      void put (std::lock_guard<std::mutex> const&, Command);
      std::list<Command> _commands;
    };
    using ScheduleQueue =
      gspc::threadsafe_interruptible_queue_with_remove
        <std::reference_wrapper<InterruptionContext>, task::ID>;

    struct RequirementsHashAndEqByClass
    {
      std::size_t operator() (Task::SingleResourceWithPreference const&) const;
      bool operator() ( Task::SingleResourceWithPreference const&
                      , Task::SingleResourceWithPreference const&
                      ) const;
    };
    template<typename T>
      using ByLeadingResourceClass
        = std::unordered_map< Task::SingleResourceWithPreference
                            , T
                            , RequirementsHashAndEqByClass
                            , RequirementsHashAndEqByClass
                            >;

    ByLeadingResourceClass<ScheduleQueue> _schedule_queues;
    CommandQueue _command_queue;

    //! state only modified by command_thread
    std::unordered_map<job::ID, resource_manager::WithPreferences::Acquired>
      _jobs;
    std::unordered_map<task::ID, job::ID> _job_by_task;
    std::unordered_map<task::ID, InterruptionContext>
      _interruption_context_by_task;
    std::unordered_map<task::ID, std::size_t> _failed_attempts;
    std::uint64_t _next_job_id {0};

    bool _stopping {false};

    // manually maintained _schedule_queue.size() + in flight in schedule thread
    std::size_t _scheduling_items {0};
    void schedule_queue_push (task::ID);
    bool schedule_queue_remove (task::ID);
    void task_back_from_schedule_queue (task::ID);
    void schedule_queues_interrupt_and_join_threads();

    ByLeadingResourceClass<std::thread> _schedule_threads;
    void schedule_thread
      ( Task::SingleResourceWithPreference
      , std::reference_wrapper<ScheduleQueue>
      );

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
