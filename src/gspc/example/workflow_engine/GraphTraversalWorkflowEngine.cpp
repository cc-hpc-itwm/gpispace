#include <gspc/example/workflow_engine/GraphTraversalWorkflowEngine.hpp>

#include <gspc/serialization.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gspc
{
  GraphTraversalWorkflowEngine::GraphTraversalWorkflowEngine
    ( boost::filesystem::path module
    , std::unordered_set<Node> open
    , Task::Symbol symbol
    , std::unordered_map<std::string, std::uint64_t> inputs
    )
  {
    _workflow_state._module = module;
    _workflow_state._open = open;
    _workflow_state._symbol = symbol;
    _workflow_state._inputs = inputs;
  }

  auto GraphTraversalWorkflowEngine::structure() const -> Forest<Node> const&
  {
    return _workflow_state._structure;
  }

  Task GraphTraversalWorkflowEngine::at (task::ID task_id) const
  {
    return _processing_state.at (task_id);
  }

  auto GraphTraversalWorkflowEngine::seen() const
    -> std::unordered_map<Node, std::size_t> const&
  {
    return _workflow_state._seen;
  }

  auto GraphTraversalWorkflowEngine::open() const
    -> std::unordered_set<Node> const&
  {
    return _workflow_state._open;
  }

  Task::Symbol const& GraphTraversalWorkflowEngine::symbol() const
  {
    return _workflow_state._symbol;
  }

  bool GraphTraversalWorkflowEngine::workflow_finished() const
  {
    return _workflow_state._got_heureka || _workflow_state._open.empty();
  }

  template<typename Archive>
    void GraphTraversalWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & _module;
    ar & _structure;
    ar & _seen;
    ar & _open;
    ar & _symbol;
    ar & _inputs;
    ar & _got_heureka;
  }

  workflow_engine::State GraphTraversalWorkflowEngine::state() const
  {
    return { bytes_save (_workflow_state)
           , workflow_finished()
           , _processing_state
           };
  }

  GraphTraversalWorkflowEngine::GraphTraversalWorkflowEngine
      (workflow_engine::State state)
    : _workflow_state (bytes_load<WorkflowState> (state.engine_specific))
    , _processing_state (state.processing_state)
  {
    if (state.workflow_finished != workflow_finished())
    {
      throw std::logic_error ("INCONSISTENCY: finished or not!?");
    }
  }

  namespace
  {
    GraphTraversalWorkflowEngine::Node pop_any
      (std::unordered_set<GraphTraversalWorkflowEngine::Node>& values)
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

    auto const& inputs (_workflow_state._inputs);

    return _processing_state.extract
      ( "core"
      , _workflow_state._symbol == "static_map"
         ? bytes_save (StaticMapInput {parent})
      : _workflow_state._symbol == "dynamic_map"
         ? bytes_save (DynamicMapInput {parent, inputs.at ("N")})
      : _workflow_state._symbol == "nary_tree"
         ? bytes_save ( NaryTreeInput { parent
                                      , inputs.at ("N")
                                      , inputs.at ("B")
                                      , inputs.at ("heureka_value")
                                      }
                      )
      : throw std::logic_error ("todo: symbol should be an enum here")
      , _workflow_state._module
      , _workflow_state._symbol
      );
  }

  interface::WorkflowEngine::InjectResult GraphTraversalWorkflowEngine::inject
    (task::ID id, task::Result result)
  {
    bool got_first_heureka {false};

    _processing_state.inject
      ( std::move (id)
      , std::move (result)
      , [&] (Task const& input_task, task::result::Success const& success)
        {
          auto mark_seen
            ( [&] (GraphTraversalOutput const& children, std::uint64_t parent)
              {
                for (auto child : children)
                {
                  if (!_workflow_state._seen[child]++)
                  {
                    _workflow_state._open.emplace (child);
                    _workflow_state._structure.insert (child, {}, {parent});
                  }
                }
              }
            );

          if (input_task.symbol == "static_map")
          {
            auto output (bytes_load<GraphTraversalOutput> (success.output));
            auto input (bytes_load<StaticMapInput> (input_task.input));

            mark_seen (output, input.parent);
          }
          else if (input_task.symbol == "dynamic_map")
          {
            auto output (bytes_load<GraphTraversalOutput> (success.output));
            auto input (bytes_load<DynamicMapInput> (input_task.input));

            mark_seen (output, input.parent);
          }
          else if (input_task.symbol == "nary_tree")
          {
            auto output
              (bytes_load<GraphTraversalOutputWithHeureka> (success.output));
            auto input (bytes_load<NaryTreeInput> (input_task.input));

            mark_seen (output.children, input.parent);

            if (output.heureka)
            {
              got_first_heureka = !_workflow_state._got_heureka;

              _workflow_state._got_heureka = true;
            }
          }
          else
          {
            throw std::logic_error ("task with symbol GTWE can't send");
          }
        }
      );

    return got_first_heureka
      ? InjectResult {{}, _processing_state.extracted()}
      : InjectResult {}
    ;
  }
}
