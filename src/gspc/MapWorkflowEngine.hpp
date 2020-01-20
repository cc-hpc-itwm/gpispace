#pragma once

#include <gspc/interface/WorkflowEngine.hpp>

#include <boost/variant.hpp>
#include <gspc/ErrorOr.hpp>
#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <cstdint>

namespace gspc
{
  class MapWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    MapWorkflowEngine (std::uint64_t);

    virtual boost::variant<Task, bool> extract() override;
    virtual InjectResult inject (task::ID, ErrorOr<task::Result>) override;

    virtual workflow_engine::State state() const override;
    MapWorkflowEngine (workflow_engine::State);

  private:
    struct WorkflowState
    {
      std::uint64_t N;
      std::uint64_t i {0};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };
}
