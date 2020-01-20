#pragma once

#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/value_type.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <boost/variant.hpp>

#include <cstdint>
#include <list>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  //! graph might contain cycles and diamonds

  //! nodes are expanded only once. duplicate parent connection is
  //! inserted by the child that returns first

  class GraphTraversalWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    GraphTraversalWorkflowEngine
      ( std::unordered_set<value_type>
      , Task::Symbol
      , Task::Inputs // default parameter for each task
      );

    Forest<value_type> const& structure() const;
    std::unordered_map<value_type, std::size_t> const& seen() const;
    std::unordered_set<value_type> const& open() const;
    Task::Symbol const& symbol() const;
    Task::Inputs const& inputs() const;

    virtual boost::variant<Task, bool> extract() override;
    virtual InjectResult inject (task::ID, ErrorOr<task::Result>) override;

    virtual workflow_engine::State state() const override;
    GraphTraversalWorkflowEngine (workflow_engine::State);

  private:
    struct WorkflowState
    {
      Forest<value_type> _structure;
      std::unordered_map<value_type, std::size_t> _seen;
      std::unordered_set<value_type> _open;

      Task::Symbol _symbol;
      Task::Inputs _inputs;

      bool _got_heureka {false};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };
}
