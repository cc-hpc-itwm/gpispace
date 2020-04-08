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
  struct MapInput
  {
    std::uint64_t N;
    std::uint64_t i;
    std::uint64_t o;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
  using MapOutput = MapInput;

  class MapWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    MapWorkflowEngine (boost::filesystem::path module, std::uint64_t);

    virtual boost::variant<task::ID, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;

    virtual workflow_engine::State state() const override;
    MapWorkflowEngine (workflow_engine::State);

    virtual Task const& at (task::ID) const override;

  private:
    struct WorkflowState
    {
      task::Implementation implementation;

      std::uint64_t N;
      std::uint64_t i {0};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };
}

#include <gspc/example/workflow_engine/MapWorkflowEngine.ipp>
