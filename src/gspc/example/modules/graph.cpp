#include <gspc/interface/module_api.hpp>

#include <gspc/example/workflow_engine/GraphTraversalWorkflowEngine.hpp>
#include <gspc/serialization.hpp>

#include <stdexcept>

namespace graph_so
{
  gspc::GraphTraversalOutput static_map (gspc::StaticMapInput)
  {
    return {};
  }

  gspc::GraphTraversalOutput dynamic_map (gspc::DynamicMapInput input)
  {
    // when parent == 0 then generate all tasks [1..N]
    // when parent != 0 then verify its less or equal to N
    if (input.parent > input.N)
    {
      throw std::logic_error
        ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
    }

    gspc::GraphTraversalOutput children;

    if (input.parent == 0)
    {
      auto n (input.N);
      while (n --> 0)
      {
        children.emplace (n + 1);
      }
    }

    return children;
  }

  gspc::GraphTraversalOutputWithHeureka nary_tree (gspc::NaryTreeInput input)
  {
    if (!(input.parent < input.N))
    {
      throw std::logic_error
        ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
    }

    gspc::GraphTraversalOutputWithHeureka output;

    auto b (input.B);
    auto const n (input.N);
    auto const parent (input.parent);

    output.heureka = parent == input.heureka_value;

    auto const child_base (b * parent + 1);

    while (b --> 0)
    {
      auto const child (child_base + b);

      if (child < n)
      {
        output.children.emplace (child);
      }
    }

    return output;
  }
}

//! SYNTAX GOAL: Replace Rest of file by:
//! GSPC_MAKE_MODULE_FUNCTIONS
//!   ( "static_map", &graph_so::static_map
//!   , "dynamic_map", &graph_so::dynamic_map
//!   , "nary_tree", &graph_so::nary_tree
//!   );

namespace autogen
{
  std::vector<char> static_map (std::vector<char> input)
  {
    return gspc::bytes_save
      (graph_so::static_map (gspc::bytes_load<gspc::StaticMapInput> (input)));
  }

  std::vector<char> dynamic_map (std::vector<char> input)
  {
    return gspc::bytes_save
      (graph_so::dynamic_map (gspc::bytes_load<gspc::DynamicMapInput> (input)));
  }

  std::vector<char> nary_tree (std::vector<char> input)
  {
    return gspc::bytes_save
      (graph_so::nary_tree (gspc::bytes_load<gspc::NaryTreeInput> (input)));
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = { {"static_map", &autogen::static_map}
    , {"dynamic_map", &autogen::dynamic_map}
    , {"nary_tree", &autogen::nary_tree}
    };
