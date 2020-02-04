#include <gspc/example/workflow_engine/ClassConflictWorkflowEngine.hpp>

#include <gspc/serialization.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gspc
{
  ClassConflictWorkflowEngine::ClassConflictWorkflowEngine
    (boost::filesystem::path module, std::size_t prefix_chain_length)
  {
    _workflow_state.implementation = {module, "identity"};
    _workflow_state.prefix_chain_length = prefix_chain_length;
  }

  bool ClassConflictWorkflowEngine::workflow_finished() const
  {
    return _workflow_state.extract == _workflow_state.prefix_chain_length + 1;
  }

  Task const& ClassConflictWorkflowEngine::at (task::ID task_id) const
  {
    return _processing_state.at (task_id);
  }

  template<typename Archive>
    void ClassConflictWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & implementation;
    ar & extract;
    ar & prefix_chain_length;
  }

  workflow_engine::State ClassConflictWorkflowEngine::state() const
  {
    return {bytes_save (_workflow_state), _processing_state};
  }

  ClassConflictWorkflowEngine::ClassConflictWorkflowEngine
    (workflow_engine::State state)
    : _workflow_state (bytes_load<WorkflowState> (state.engine_specific))
    , _processing_state (state.processing_state)
  {}

  boost::variant<task::ID, bool> ClassConflictWorkflowEngine::extract()
  {
    if (workflow_finished())
    {
      return !_processing_state.has_extracted_tasks();
    }

    resource::Class const resource_class
      ( _workflow_state.extract < _workflow_state.prefix_chain_length
      ? "A"
      : "B"
      );

    ++_workflow_state.extract;

    return _processing_state.extract
      ( Task::SingleResource {resource_class, _workflow_state.implementation}
      , {}
      );
  }

  interface::WorkflowEngine::InjectResult
    ClassConflictWorkflowEngine::inject (task::ID id, task::Result result)
  {
    bool heureka {false};

    _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [&] (Task const& task, task::result::Success const&)
        {
          heureka = fhg::util::visit<bool>
            ( task.requirements
            , [] (Task::SingleResource const& single_resource)
              {
                return single_resource.first == "B";
              }
            , [] (auto const&)
              {
                return false;
              }
            );
        }
      );

    return heureka
      ? InjectResult {{}, _processing_state.extracted()}
      : InjectResult {}
    ;
  }
}
