#include <gspc/module_api.hpp>

#include <stdexcept>

namespace graph_so
{
  gspc::task::result::Success::Outputs static_map
    (gspc::Task::Inputs inputs)
  {
    // do not generate children
    if (  inputs.size() != 1
       || inputs.count ("parent") != 1
       )
    {
      throw std::logic_error
        ("Worker::execute: Graph: Static Map: Corrupted task.");
    }

    return inputs;
  }

  gspc::task::result::Success::Outputs dynamic_map
    (gspc::Task::Inputs inputs)
  {
    auto value_at
      ( [&] (auto key)
        {
          return inputs.equal_range (key).first->second;
        }
      );

    // when parent == 0 then generate all tasks [1..N]
    // when parent != 0 then verify its less or equal to N
    if (  inputs.size() != 2
       || inputs.count ("parent") != 1
       || inputs.count ("N") != 1
       || value_at ("parent") > value_at ("N")
       )
    {
      throw std::logic_error
        ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
    }

    auto outputs (inputs);

    auto const parent (value_at ("parent"));

    if (parent == 0)
    {
      auto n (value_at ("N"));

      while (n --> 0)
      {
        outputs.emplace ("children", n + 1);
      }
    }

    return outputs;
  }

  gspc::task::result::Success::Outputs nary_tree
    (gspc::Task::Inputs inputs)
  {
    auto value_at
      ( [&] (auto key)
        {
          return inputs.equal_range (key).first->second;
        }
      );

    if (  inputs.count ("parent") != 1
       || inputs.count ("N") != 1
       || inputs.count ("B") != 1
       || !(value_at ("parent") < value_at ("N"))
       )
    {
      throw std::logic_error
        ("Worker::execute: Graph: Dynamic Map: Corrupted task.");
    }

    auto outputs (inputs);

    auto b (value_at ("B"));
    auto const n (value_at ("N"));
    auto const parent (value_at ("parent"));

    outputs.emplace
      ( "heureka"
      , inputs.count ("heureka_value")
      && parent == value_at ("heureka_value")
      );

    auto const child_base (b * parent + 1);

    while (b --> 0)
    {
      auto const child (child_base + b);

      if (child < n)
      {
        outputs.emplace ("children", child);
      }
    }

    return outputs;
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = { {"static_map", &graph_so::static_map}
    , {"dynamic_map", &graph_so::dynamic_map}
    , {"nary_tree", &graph_so::nary_tree}
    };
