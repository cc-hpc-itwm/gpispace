#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Task.hpp>
#include <gspc/interface/WorkflowEngine.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <boost/variant.hpp>

namespace gspc
{
  class PetriNetWorkflow{};
  class PetriNetWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    PetriNetWorkflowEngine (PetriNetWorkflow);

    virtual boost::variant<Task, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;
  };
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class ReschedulingGreedyScheduler;
  //! - schedule_queues needs a guard for at+emplace+iterate in stop
  //! - cancel_task(id) needs resource class again (needs storing
  //!   task, or query wfe?!)
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    // class CoallocationWithPreference : public interface::ResourceManager
    // {
    //   // `[(rc, count)]` or `[rc], count` or both?
    //   Acquired acquire (std::list<std::pair<resource::Class, std::size_t>>);
    // };
  }
}
