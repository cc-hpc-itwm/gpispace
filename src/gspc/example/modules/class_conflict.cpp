#include <gspc/interface/module_api.hpp>

namespace class_conflict
{
  std::vector<char> identity (std::vector<char> input)
  {
    return input;
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = {{"identity", &class_conflict::identity}};
