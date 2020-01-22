#pragma once

#include <util-generic/dllexport.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace gspc
{
  using ModuleFunction = std::vector<char> (*) (std::vector<char>);
  using ModuleFunctions = std::unordered_map<std::string, ModuleFunction>;
}

//! \todo Macro?
extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions;
