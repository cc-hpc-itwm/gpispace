#pragma once

#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>

#include <cstdint>
#include <list>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  struct StaticMapInput
  {
    std::uint64_t parent;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
  struct DynamicMapInput
  {
    std::uint64_t parent;
    std::uint64_t N;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
  using GraphTraversalOutput = std::unordered_set<std::uint64_t>;
  struct NaryTreeInput
  {
    std::uint64_t parent;
    std::uint64_t N;
    std::uint64_t B;
    std::uint64_t heureka_value;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
  struct GraphTraversalOutputWithHeureka
  {
    GraphTraversalOutput children;
    bool heureka;

    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };

  //! graph might contain cycles and diamonds

  //! nodes are expanded only once. duplicate parent connection is
  //! inserted by the child that returns first

  class GraphTraversalWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    using Node = std::uint64_t;

    GraphTraversalWorkflowEngine
      ( boost::filesystem::path module
      , std::unordered_set<Node>
      , Task::Symbol
      , std::unordered_map<std::string, Node> // default parameter for each task
      );

    Forest<Node> const& structure() const;
    std::unordered_map<Node, std::size_t> const& seen() const;
    std::unordered_set<Node> const& open() const;
    Task::Symbol const& symbol() const;

    virtual boost::variant<Task, bool> extract() override;
    virtual InjectResult inject (task::ID, task::Result) override;

    virtual workflow_engine::State state() const override;
    GraphTraversalWorkflowEngine (workflow_engine::State);

    virtual Task at (task::ID) const override;

  private:
    struct WorkflowState
    {
      boost::filesystem::path _module;

      Forest<Node> _structure;
      std::unordered_map<Node, std::size_t> _seen;
      std::unordered_set<Node> _open;

      Task::Symbol _symbol;
      std::unordered_map<std::string, Node> _inputs;

      bool _got_heureka {false};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */);
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };

  template<typename Archive>
    void StaticMapInput::serialize (Archive& ar, unsigned int)
  {
    ar & parent;
  }
  template<typename Archive>
    void DynamicMapInput::serialize (Archive& ar, unsigned int)
  {
    ar & parent;
    ar & N;
  }
  template<typename Archive>
    void NaryTreeInput::serialize (Archive& ar, unsigned int)
  {
    ar & parent;
    ar & N;
    ar & B;
    ar & heureka_value;
  }
  template<typename Archive>
    void GraphTraversalOutputWithHeureka::serialize (Archive& ar, unsigned int)
  {
    ar & children;
    ar & heureka;
  }
}
