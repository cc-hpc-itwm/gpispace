#include <gspc/GraphTraversalWorkflowEngine.hpp>

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
  GraphTraversalWorkflowEngine::GraphTraversalWorkflowEngine
    ( std::unordered_set<value_type> open
    , Task::Symbol symbol
    , Task::Inputs inputs
    )
  {
    _workflow_state._open = open;
    _workflow_state._symbol = symbol;
    _workflow_state._inputs = inputs;
  }

  Forest<value_type> const& GraphTraversalWorkflowEngine::structure() const
  {
    return _workflow_state._structure;
  }

  std::unordered_map<value_type, std::size_t> const&
    GraphTraversalWorkflowEngine::seen() const
  {
    return _workflow_state._seen;
  }

  std::unordered_set<value_type> const&
    GraphTraversalWorkflowEngine::open() const
  {
    return _workflow_state._open;
  }

  Task::Symbol const& GraphTraversalWorkflowEngine::symbol() const
  {
    return _workflow_state._symbol;
  }

  Task::Inputs const& GraphTraversalWorkflowEngine::inputs() const
  {
    return _workflow_state._inputs;
  }

  bool GraphTraversalWorkflowEngine::workflow_finished() const
  {
    return _workflow_state._got_heureka || _workflow_state._open.empty();
  }

  template<typename Archive>
    void GraphTraversalWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & _structure;
    ar & _seen;
    ar & _open;
    ar & _symbol;
    ar & _inputs;
    ar & _got_heureka;
  }

  workflow_engine::State GraphTraversalWorkflowEngine::state() const
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

  GraphTraversalWorkflowEngine::GraphTraversalWorkflowEngine
      (workflow_engine::State state)
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

  namespace
  {
    value_type pop_any (std::unordered_set<value_type>& values)
    {
      auto selected (values.begin());
      auto value (*selected);
      values.erase (selected);
      return value;
    }
  }

  boost::variant<Task, bool> GraphTraversalWorkflowEngine::extract()
  {
    if (workflow_finished())
    {
      return !_processing_state.has_extracted_tasks();
    }

    auto const parent (pop_any (_workflow_state._open));
    if (!_workflow_state._seen[parent]++)
    {
      _workflow_state._structure.insert (parent, {}, {});
    }

    Task::Inputs inputs (_workflow_state._inputs);
    inputs.emplace ("parent", parent);

    return _processing_state.extract
      ("core", {{0}}, inputs, "graph_so", _workflow_state._symbol);
  }

  void GraphTraversalWorkflowEngine::inject
    (task::ID id, ErrorOr<task::Result> result)
  {
    return _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [&] (Task const& input_task, task::Result const& result)
        {
          auto const& outputs (result.outputs);
          auto const& inputs (input_task.inputs);

          auto value_at
            ( [&] (auto const& xs, auto x)
              {
                return xs.equal_range (x).first->second;
              }
            );

          if (  outputs.count ("parent") != 1
             || inputs.count ("parent") != 1
             || value_at (outputs, "parent") != value_at (inputs, "parent")
             || (  result.heureka_group
                && !(*result.heureka_group == heureka::Group {0})
                )
             )
          {
            throw std::logic_error
              ("GraphTraversalWorkflowEngine::inject: Unexpected result");
          }

          auto const& parent (value_at (outputs, "parent"));
          auto children (outputs.equal_range ("children"));

          for (auto child (children.first); child != children.second; ++child)
          {
            if (!_workflow_state._seen[child->second]++)
            {
              _workflow_state._open.emplace (child->second);
              _workflow_state._structure.insert (child->second, {}, {parent});
            }
          }

          if (result.heureka_group)
          {
            _workflow_state._got_heureka = true;
          }
        }
      );
  }
}