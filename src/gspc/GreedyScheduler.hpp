#pragma once

#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/comm/scheduler/workflow_engine/Client.hpp>
#include <gspc/heureka/Group.hpp>
#include <gspc/interface/Scheduler.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>
#include <gspc/resource_manager/Trivial.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>

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
    //! \todo ~GreedyScheduler -> wait()

    virtual void wait() override;
    virtual void stop() override;
    virtual void finished (job::ID, job::FinishReason) override;

  private:
    comm::scheduler::workflow_engine::Client _workflow_engine;
    resource_manager::Trivial& _resource_manager;
    ScopedRuntimeSystem& _runtime_system;

    std::atomic<bool> _stopped {false};

    std::mutex _guard_state;
    std::condition_variable _injected_or_stopped;
    bool _injected {false};
    std::unordered_map<job::ID, resource::ID> _jobs;

    using HeurekaGroups = boost::bimap
      < boost::bimaps::unordered_multiset_of<heureka::Group>
      , boost::bimaps::unordered_set_of<job::ID>
      >;

    HeurekaGroups _heureka_groups;
    std::uint64_t _next_job_id {0};

    std::thread _thread;
    void scheduling_thread();

    template<typename Function>
      void do_worker_call (resource::ID, job::ID, Function&& function) noexcept;
  };
}