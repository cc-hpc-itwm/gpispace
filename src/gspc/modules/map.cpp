#include <gspc/module_api.hpp>

#include <stdexcept>

namespace map_so
{
  gspc::task::result::Success::Outputs identity
    (gspc::Task::Inputs inputs)
  {
    auto value_at
      ( [&] (auto key)
        {
          return inputs.equal_range (key).first->second;
        }
      );

    if (  inputs.size() != 3
       || inputs.count ("input") != 1
       || inputs.count ("output") != 1
       || inputs.count ("N") != 1
       || value_at ("input") + value_at ("output") != value_at ("N")
       )
    {
      throw std::logic_error ("Worker::execute: Map: Corrupted task.");
    }

    return inputs;
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = {{"identity", &map_so::identity}};
