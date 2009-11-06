#ifndef SDPA_MODULE_MODULE_LOADER_HPP
#define SDPA_MODULE_MODULE_LOADER_HPP 1

#include <map>
#include <string>

//#include <sdpa/sdpa-config.hpp>

// choose the implementation depending on what we have available
#if defined(HAVE_FVM)
#   include <sdpa/modules/FVMModuleLoader.hpp>

namespace sdpa {
namespace modules {
  typedef FVMModuleLoader ModuleLoader;
}}

#else // provide a fallback implementation
//#   include <sdpa/modules/FallBackModuleLoader.hpp>
#   include <FallBackModuleLoader.hpp>

namespace sdpa {
namespace modules {
  typedef FallBackModuleLoader ModuleLoader;
}}

#endif

#endif
