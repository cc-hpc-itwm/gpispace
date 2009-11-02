#ifndef SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP
#define SDPA_MODULES_FALL_BACK_MODULE_LOADER_HPP 1

#include <map>
#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/modules/exceptions.hpp>
#include <sdpa/modules/Module.hpp>

namespace sdpa {
namespace modules {
  class FallBackModuleLoader {
  public:
    typedef shared_ptr<FallBackModuleLoader> ptr_t;

    static ptr_t create() { return ptr_t(new FallBackModuleLoader()); }

    typedef std::map<std::string, Module::ptr_t> module_table_t;

    ~FallBackModuleLoader();

    const Module& get(const std::string &module) const throw(ModuleNotLoaded);
    Module& get(const std::string &module) throw(ModuleNotLoaded) {
      return const_cast<Module&>(const_cast<const FallBackModuleLoader*>(this)->get(module));
    }

    Module& load(const std::string &file) throw(ModuleLoadFailed);
    void unload(const std::string &module);
  private:
    FallBackModuleLoader()
      : module_table_()
    {
    }
    FallBackModuleLoader(const FallBackModuleLoader&);
    void unload_all();
    void unload(module_table_t::iterator);

    module_table_t module_table_;
  };
}}

#endif
