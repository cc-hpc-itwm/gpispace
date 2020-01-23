#include <gspc/example/workflow_engine/MapWorkflowEngine.hpp>

#include <gspc/serialization.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gspc
{
  MapWorkflowEngine::MapWorkflowEngine
    (boost::filesystem::path module, std::uint64_t N)
  {
    _workflow_state.module = module;
    _workflow_state.N = N;
  }

  bool MapWorkflowEngine::workflow_finished() const
  {
    return !(_workflow_state.i < _workflow_state.N);
  }

  Task MapWorkflowEngine::at (task::ID task_id) const
  {
    return _processing_state.at (task_id);
  }

  template<typename Archive>
    void MapWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & module;
    ar & N;
    ar & i;
  }

  workflow_engine::State MapWorkflowEngine::state() const
  {
    return { bytes_save (_workflow_state)
           , workflow_finished()
           , _processing_state
           };
  }

  MapWorkflowEngine::MapWorkflowEngine (workflow_engine::State state)
    : _workflow_state (bytes_load<WorkflowState> (state.engine_specific))
    , _processing_state (state.processing_state)
  {
    if (state.workflow_finished != workflow_finished())
    {
      throw std::logic_error ("INCONSISTENCY: finished or not!?");
    }
  }

  boost::variant<Task, bool> MapWorkflowEngine::extract()
  {
    if (workflow_finished())
    {
      return !_processing_state.has_extracted_tasks();
    }

    ++_workflow_state.i;

    return _processing_state.extract
      ( "core"
      , bytes_save ( MapInput { _workflow_state.N
                              , _workflow_state.i
                              , _workflow_state.N - _workflow_state.i
                              }
                   )
      , _workflow_state.module
      , "identity"
      );
  }

  interface::WorkflowEngine::InjectResult
    MapWorkflowEngine::inject (task::ID id, task::Result result)
  {
    _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [] (Task const& input_task, task::result::Success const& success)
        {
          if (input_task.input != success.output)
          {
            throw std::logic_error
              ("MapWorkflowEngine::inject: Unexpected outputs.");
          }
        }
      );

    return {};
  }
}
