#ifndef SDPA_MODULES_MODULES_HPP
#define SDPA_MODULES_MODULES_HPP 1

#include <sdpa/modules/ModuleLoader.hpp>

namespace sdpa { namespace modules {
  ModuleLoader::ptr_t createModuleLoader() {
    return ModuleLoader::create();
  }  
}}

#endif
