#pragma once

#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>

#include <cstdint>

namespace gspc
{
  class ClassConflictWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    ClassConflictWorkflowEngine
      (boost::filesystem::path, std::size_t prefix_chain_length);

    virtual boost::variant<task::ID, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;

    virtual workflow_engine::State state() const override;
    ClassConflictWorkflowEngine (workflow_engine::State);

    virtual Task const& at (task::ID) const override;

  private:
    struct WorkflowState
    {
      task::Implementation implementation;
      std::size_t extract {0};
      std::size_t prefix_chain_length;

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };
}
