#ifndef SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP
#define SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP 1

#include <map>
#include <string>
#include <list>
#include <cassert>
#include <dlfcn.h>

#include <sdpa/memory.hpp>
#include <sdpa/modules/exceptions.hpp>
#include <sdpa/modules/types.hpp>
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

      /*
       * this does not work with -pedantic i think, so we'll work around it
       *    sdpa::modules::InitFunction init = reinterpret_cast<sdpa::modules::InitFunction>((dlsym(handle, "sdpa_mod_init")));
       */
      sdpa::modules::InitFunction init (NULL);
      {
        struct {
          union {
            void *symbol;
            InitFunction function;
          };
        } func_ptr;
        func_ptr.symbol = dlsym(handle, "sdpa_mod_init");
        init = func_ptr.function;
      }

      if ( (error = dlerror()) != NULL || init == NULL) {
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

      LOG(INFO, "sucessfully loaded file: " << file << " with module: " << *mod);
      return *mod;
    }

    void unload(const std::string &module)
    {
      module_table_t::iterator mod = module_table_.find(module);
      if (mod != module_table_.end()) {
        unload(mod);
      }
    }

    void writeTo(std::ostream &os) const
    {
      os << "[";

      module_table_t::const_iterator m(module_table_.begin());
      while (m != module_table_.end())
      {
        os << (*(m->second));
        ++m;

        if (m != module_table_.end())
          os << ", ";
      }

      os << "]";
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

inline std::ostream &operator<<(std::ostream &os, const sdpa::modules::FallBackModuleLoader &loader)
{
  loader.writeTo(os);
  return os;
}

#endif
