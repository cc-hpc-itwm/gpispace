#include <gspc/example/workflow_engine/SequenceWorkflowEngine.hpp>

#include <gspc/serialization.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gspc
{
  SequenceWorkflowEngine::SequenceWorkflowEngine
    (boost::filesystem::path module, std::uint64_t N)
  {
    _workflow_state.implementation = {module, "identity"};
    _workflow_state.N = N;
  }

  bool SequenceWorkflowEngine::workflow_finished() const
  {
    return !(_workflow_state.i < _workflow_state.N);
  }

  Task SequenceWorkflowEngine::at (task::ID task_id) const
  {
    return _processing_state.at (task_id);
  }

  template<typename Archive>
    void SequenceWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & implementation;
    ar & N;
    ar & i;
  }

  workflow_engine::State SequenceWorkflowEngine::state() const
  {
    return { bytes_save (_workflow_state)
           , workflow_finished()
           , _processing_state
           };
  }

  SequenceWorkflowEngine::SequenceWorkflowEngine (workflow_engine::State state)
    : _workflow_state (bytes_load<WorkflowState> (state.engine_specific))
    , _processing_state (state.processing_state)
  {
    if (state.workflow_finished != workflow_finished())
    {
      throw std::logic_error ("INCONSISTENCY: finished or not!?");
    }
  }

  boost::variant<Task, bool> SequenceWorkflowEngine::extract()
  {
    if (workflow_finished())
    {
      return !_processing_state.has_extracted_tasks();
    }

    if (!_workflow_state.step)
    {
      return false;
    }

    ++_workflow_state.i;
    _workflow_state.step = false;


    return _processing_state.extract
      ( Task::SingleResource {"core", _workflow_state.implementation}
      , bytes_save (SequenceInput {_workflow_state.i})
      );
  }

  interface::WorkflowEngine::InjectResult
    SequenceWorkflowEngine::inject (task::ID id, task::Result result)
  {
    _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [&] (Task const& input_task, task::result::Success const& success)
        {
          if (input_task.input != success.output)
          {
            throw std::logic_error
              ("SequenceWorkflowEngine::inject: Unexpected outputs.");
          }

          _workflow_state.step = true;
        }
      );

    return {};
  }
}
