#ifndef SDPA_MODULE_MODULE_LOADER_HPP
#define SDPA_MODULE_MODULE_LOADER_HPP 1

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

// choose the implementation depending on what we have available
#if defined(HAVE_FVM)
#   include <sdpa/modules/FVMModuleLoader.hpp>

namespace sdpa {
namespace modules {
  typedef FVMModuleLoader ModuleLoader;
}}

#else // provide a fallback implementation
#   include <sdpa/modules/FallBackModuleLoader.hpp>

namespace sdpa {
namespace modules {
  typedef FallBackModuleLoader ModuleLoader;
}}

#endif

#endif
