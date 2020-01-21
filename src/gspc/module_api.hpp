#pragma once

//! \todo use vector<char>/string, remove include to gspc/
#include <gspc/Task.hpp>
#include <gspc/task/Result.hpp>

#include <util-generic/dllexport.hpp>

#include <string>
#include <unordered_map>

namespace gspc
{
  using ModuleFunction = task::result::Success::Outputs (*) (Task::Inputs);
  using ModuleFunctions = std::unordered_map<std::string, ModuleFunction>;
}

//! \todo Macro?
extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions;
