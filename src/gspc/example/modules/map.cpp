#include <gspc/interface/module_api.hpp>

#include <gspc/example/workflow_engine/MapWorkflowEngine.hpp>
#include <gspc/serialization.hpp>

#include <stdexcept>

namespace map_so
{
  gspc::MapOutput identity (gspc::MapInput input)
  {
    if (input.i + input.o != input.N)
    {
      throw std::logic_error ("Worker::execute: Map: Corrupted task.");
    }

    return input;
  }
}

//! \todo automatically generate wrapper
namespace autogen
{
  std::vector<char> identity (std::vector<char> input)
  {
    return gspc::bytes_save
      (map_so::identity (gspc::bytes_load<gspc::MapInput> (input)));
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = {{"identity", &/*eigentlich map_so::*/autogen::identity}};
