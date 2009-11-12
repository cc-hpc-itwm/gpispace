#ifndef SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP
#define SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP 1

#include <map>
#include <string>
#include <list>
#include <cassert>
#include <dlfcn.h>

#include <sdpa/memory.hpp>
#include <sdpa/modules/exceptions.hpp>
#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

namespace sdpa {
namespace modules {
  class FallBackModuleLoader {
  public:
    typedef shared_ptr<FallBackModuleLoader> ptr_t;

    static ptr_t create() { return ptr_t(new FallBackModuleLoader()); }

    typedef std::map<std::string, Module::ptr_t> module_table_t;

    ~FallBackModuleLoader()
    {
      try {
        unload_all();
      } catch (const std::exception &ex) {
        LOG(ERROR, "could not unload module: " << ex.what());
      }
    }

    const Module& get(const std::string &module) const throw(ModuleNotLoaded)
    {
      module_table_t::const_iterator mod = module_table_.find(module);
      if (mod != module_table_.end()) {
        return *mod->second;
      } else {
        throw ModuleNotLoaded(module);
      }
    }

    Module& get(const std::string &module) throw(ModuleNotLoaded)
    {
      return const_cast<Module&>(const_cast<const FallBackModuleLoader*>(this)->get(module));
    }

    Module& load(const std::string &file) throw(ModuleLoadFailed)
    {
      LOG(DEBUG, "attempting to load module from file " << file);

      char *error = 0;

#if defined(SDPA_MODULE_LOADER_BIND_NOW)
      Module::handle_t handle = dlopen(file.c_str(), RTLD_NOW | RTLD_GLOBAL);
#else
      Module::handle_t handle = dlopen(file.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
      if (! handle) {
        throw ModuleLoadFailed(dlerror(), "[name-not-set]", file);
      }

      // clear any errors
      dlerror();

      Module::ptr_t mod(new Module(handle));

      Module::InitFunction init = (Module::InitFunction)(dlsym(handle, "sdpa_mod_init"));
      if ( (error = dlerror()) != NULL) {
        dlclose(handle);
        LOG(ERROR, "module not loaded: " << error);
        throw ModuleLoadFailed(error, "[name-not-set]", file);
      }

      try {
        init(mod.get());
      } catch (const std::exception &ex) {
        dlclose(handle);
        LOG(ERROR, "errors during initialization function: " << ex.what());
        throw ModuleLoadFailed("error during mod-init function: " + std::string(ex.what()), mod->name(), file);
      } catch (...) {
        dlclose(handle);
        LOG(ERROR, "unknown error during initialization function");
        throw ModuleLoadFailed("unknown error during mod-init function", mod->name(), file);
      }

      if (mod->name().empty())
      {
        dlclose(handle);
        LOG(ERROR, "init function did not set module's name");
        throw ModuleLoadFailed("the module's initialization function did not set the name of the module!", "[name-not-set]", file);
      }

      std::pair<module_table_t::iterator, bool> insert_result = module_table_.insert(std::make_pair(mod->name(), mod));

      if (! insert_result.second)
      {
        dlclose(handle);
        LOG(ERROR, "the module \"" << mod->name() << "\" could not be inserted, a module with that name already exists!");
        throw ModuleLoadFailed("module already registered", mod->name(), file);
      }

      LOG(INFO, "sucessfully loaded: " << mod->name() << " from file " << file);
      DLOG(DEBUG, *mod);
      return *mod;
    }

    void unload(const std::string &module)
    {
      module_table_t::iterator mod = module_table_.find(module);
      if (mod != module_table_.end()) {
        unload(mod);
      }
    }

    std::list<std::string> loaded_modules() const
    {
      std::list<std::string> mods;
      for (module_table_t::const_iterator m(module_table_.begin()); m != module_table_.end(); ++m)
      {
        mods.push_back(m->first);
      }
      return mods;
    }
  private:
    FallBackModuleLoader()
      : module_table_()
    {
    }

    FallBackModuleLoader(const FallBackModuleLoader&);
    void unload_all()
    {
      while (! module_table_.empty()) {
        unload(module_table_.begin());
     }
    }

    void unload(module_table_t::iterator mod)
    {
      assert( mod != module_table_.end() );
      LOG(INFO, "unloading module " << mod->second->name());
      dlclose(mod->second->handle());
      module_table_.erase(mod);
    }

    module_table_t module_table_;
  };
}}

#endif
