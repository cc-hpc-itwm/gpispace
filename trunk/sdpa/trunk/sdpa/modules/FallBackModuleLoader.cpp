#include "FallBackModuleLoader.hpp"

#include <cassert>
#include <iostream>
#include <dlfcn.h>

using namespace sdpa::modules;

FallBackModuleLoader::~FallBackModuleLoader() {
  try {
    unload_all();
  } catch (...) {
    std::cerr << "E: " << __FILE__ << ":" << __LINE__ << " - " << "could not unload module" << std::endl;
  }
}

void FallBackModuleLoader::unload(const std::string &module) {
  module_table_t::iterator mod = module_table_.find(module);
  if (mod != module_table_.end()) {
    unload(mod);
  }
}

void FallBackModuleLoader::unload(module_table_t::iterator mod) {
  assert( mod != module_table_.end() );
  dlclose(mod->second->handle());
  module_table_.erase(mod);
}

void FallBackModuleLoader::unload_all() {
  while (! module_table_.empty()) {
    unload(module_table_.begin());
  }
}

const Module& FallBackModuleLoader::get(const std::string &module) const throw(ModuleNotLoaded) {
  module_table_t::const_iterator mod = module_table_.find(module);
  if (mod != module_table_.end()) {
    return *mod->second;
  } else {
    throw ModuleNotLoaded(module);
  }
}

Module& FallBackModuleLoader::load(const std::string &module, const std::string &file) throw (ModuleLoadFailed) {
  char *error = 0;

  Module::handle_t handle = dlopen(file.c_str(), RTLD_LAZY);
  if (! handle) {
    throw ModuleLoadFailed(dlerror(), module, file);
  }

  // clear any errors
  dlerror();

  Module::ptr_t mod(new Module(module, handle));

  Module::InitFunction init = (Module::InitFunction)(dlsym(handle, "sdpa_mod_init"));
  if ( (error = dlerror()) != NULL) {
    dlclose(handle);
    throw ModuleLoadFailed(error, module, file);
  }

  try {
    init(mod.get());
  } catch (...) {
    dlclose(handle);
    throw ModuleLoadFailed("error during mod-init function", module, file);
  }

  module_table_.insert(std::make_pair(module, mod));
  return *mod;
}
