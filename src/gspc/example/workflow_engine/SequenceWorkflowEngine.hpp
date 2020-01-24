#pragma once

#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/ErrorOr.hpp>
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
  struct SequenceInput
  {
    std::uint64_t i;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
  using SequenceOutput = SequenceInput;

  class SequenceWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    SequenceWorkflowEngine (boost::filesystem::path module, std::uint64_t);

    virtual boost::variant<Task, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;

    virtual workflow_engine::State state() const override;
    SequenceWorkflowEngine (workflow_engine::State);

    virtual Task at (task::ID) const override;

  private:
    struct WorkflowState
    {
      task::Implementation implementation;

      std::uint64_t N;
      std::uint64_t i {0};
      bool step {true};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };

  template<typename Archive>
    void SequenceInput::serialize (Archive& ar, unsigned int)
  {
    ar & i;
  }
}
