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

    virtual boost::variant<task::ID, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;

    virtual Task const& at (task::ID) const override;
  };

  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;
}
