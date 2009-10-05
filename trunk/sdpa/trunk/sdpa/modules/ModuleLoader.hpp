#ifndef SDPA_MODULE_MODULE_LOADER_HPP
#define SDPA_MODULE_MODULE_LOADER_HPP 1

#include <map>
#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/modules/exceptions.hpp>
#include <sdpa/modules/Module.hpp>

namespace sdpa {
namespace modules {
  class ModuleLoader {
  public:
    typedef sdpa::shared_ptr<ModuleLoader> Ptr;

    static Ptr create() { return Ptr(new ModuleLoader()); }

    typedef std::map<std::string, sdpa::modules::Module::Ptr> module_table_t;

    ~ModuleLoader();

    const Module& get(const std::string &module) const throw(ModuleNotLoaded);
    Module& get(const std::string &module) throw(ModuleNotLoaded) {
      return const_cast<Module&>(const_cast<const ModuleLoader*>(this)->get(module));
    }

    Module& load(const std::string &module, const std::string &file) throw(ModuleLoadFailed);
    void unload(const std::string &module);
  private:
    ModuleLoader()
      : module_table_()
    {
    }
    ModuleLoader(const ModuleLoader&);
    void unload_all();
    void unload(module_table_t::iterator);

    module_table_t module_table_;
  };
}}

/*
// choose the implementation depending on what we have available
#if HAVE_FVM
#   include <sdpa/module/FVMModuleLoader.hpp>

namespace sdpa {
namespace module {
  typedef sdpa::module::FVMModuleLoader ModuleLoader;
}}

#else // provide a fallback implementation
#   include <sdpa/module/FallBackModuleLoader.hpp>

namespace sdpa {
namespace module {
  typedef sdpa::module::FallBackModuleLoader ModuleLoader;
}}

#endif
*/

#endif
