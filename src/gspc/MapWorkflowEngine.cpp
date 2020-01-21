#include <gspc/MapWorkflowEngine.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

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
    std::vector<char> data;
    {
      boost::iostreams::filtering_ostream zos
        (boost::iostreams::back_inserter (data));
      boost::archive::binary_oarchive oa (zos);

      oa & _workflow_state;
    }

    return {std::move (data), workflow_finished(), _processing_state};
  }

  MapWorkflowEngine::MapWorkflowEngine (workflow_engine::State state)
    : _processing_state (state.processing_state)
  {
    //! \todo see aloma::core::data::serialization
    auto const& data (state.engine_specific);

    boost::iostreams::filtering_istream zis
      (boost::iostreams::array_source (data.data(), data.size()));
    boost::archive::binary_iarchive ia (zis);

    ia & _workflow_state;

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

    Task::Inputs const inputs
      { {"input", _workflow_state.i}
      , {"output", _workflow_state.N - _workflow_state.i}
      , {"N", _workflow_state.N}
      };

    return _processing_state.extract
      ("core", inputs, _workflow_state.module, "identity");
  }

  interface::WorkflowEngine::InjectResult
    MapWorkflowEngine::inject (task::ID id, task::Result result)
  {
    _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [] (Task const& input_task, task::result::Success const& success)
        {
          if (input_task.inputs != success.outputs)
          {
            throw std::logic_error
              ("MapWorkflowEngine::inject: Unexpected outputs.");
          }
        }
      );

    return {};
  }
}
