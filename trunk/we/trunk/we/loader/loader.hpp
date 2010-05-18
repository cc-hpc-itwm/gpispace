#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <boost/unordered_map.hpp>
#include <string>
#include <cassert>
#include <dlfcn.h>

#include <we/loader/exceptions.hpp>
#include <we/loader/types.hpp>
#include <we/loader/Module.hpp>

#include <boost/shared_ptr.hpp>

namespace we {
  namespace loader {
    using boost::shared_ptr;

    class ModuleLoader {
    public:
      typedef shared_ptr<Module> module_ptr_t;
      typedef shared_ptr<ModuleLoader> ptr_t;

      static ptr_t create() { return ptr_t(new ModuleLoader()); }

      typedef boost::unordered_mapmap<std::string, module_ptr_t> module_table_t;

      ~ModuleLoader()
      {
        try {
          unload_all();
        } catch (const std::exception &ex) {
          LOG(ERROR, "could not unload module: " << ex.what());
        }
      }

      const module_ptr_t & get(const std::string &module) const throw(ModuleNotLoaded)
      {
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return *mod->second;
        } else {
          throw ModuleNotLoaded(module);
        }
      }

      module_ptr_t & get(const std::string &module) throw(ModuleNotLoaded)
      {
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return *mod->second;
        } else {
          return load(module);
        }
      }

      module_ptr_t & load ( const std::string & module_name ) throw (ModuleLoadFailed)
      {
        //    file_name = "lib" + module_name + ".so"
        // iterate over search path
        //    path = prefix + file_name
        return load (module_name, "./lib"+module_name+".so");
      }

      module_ptr_t & load( const std::string & module_name
                         , const std::string & path
                         ) throw(ModuleLoadFailed)
      {
        char *error = 0;
        Module::handle_t handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (! handle)
        {
          throw ModuleLoadFailed( std::string("could not load module '")
                                + module_name
                                + "' from '"+ path + "': "
                                + std::string(dlerror())
                                , module_name
                                , path
                                );
        }

        // clear any errors
        dlerror();

        /*
         * this does not work with -pedantic i think, so we'll work around it
         *    sdpa::modules::InitFunction init = reinterpret_cast<sdpa::modules::InitFunction>((dlsym(handle, "sdpa_mod_init")));
         */
        we::loader::InitializeFunction init (NULL);
        {
          struct
          {
            union
            {
              void *symbol;
              InitializeFunction function;
            };
          } func_ptr;

          func_ptr.symbol = dlsym(handle, "we_mod_initialize");
          init = func_ptr.function;
        }

        if ( (error = dlerror()) != NULL || init == NULL) {
          dlclose(handle);
          throw ModuleLoadFailed( "could not lookup intialize function: " + std::string(error)
                                , module_name
                                , path
                                );
        }

        module_ptr_t mod(new Module(module_name, handle));

        try {
          init(mod.get());
        } catch (const std::exception &ex) {
          dlclose(handle);
          throw ModuleLoadFailed("error during mod-init function: " + std::string(ex.what()), mod->name(), path);
        } catch (...) {
          dlclose(handle);
          throw ModuleLoadFailed("unknown error during mod-init function", mod->name(), path);
        }

        if (mod->name().empty())
        {
          dlclose(handle);
          throw ModuleLoadFailed("the module's initialization function did not set the name of the module!", module_name, path);
        }

        std::pair<module_table_t::iterator, bool> insert_result = module_table_.insert(std::make_pair(mod->name(), mod));

        if (! insert_result.second)
        {
          dlclose(handle);
          throw ModuleLoadFailed("module already registered", mod->name(), path);
        }

        return mod;
      }

      void unload(const std::string &module_name)
      {
        module_table_t::iterator mod = module_table_.find(module_name);
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
      ModuleLoader()
        : module_table_()
      {}

      ModuleLoader(const ModuleLoader&);
      ModuleLoader & operator = (const ModuleLoader &);

      void unload_all()
      {
        while (! module_table_.empty()) {
          unload(module_table_.begin());
        }
      }

      void unload(module_table_t::iterator mod)
      {
        assert(mod != module_table_.end());
        module_table_.erase(mod);
      }

      module_table_t module_table_;
    };

    inline std::ostream &operator<< ( std::ostream &os
                                    , const ModuleLoader &loader
                                    )
    {
      loader.writeTo(os);
      return os;
    }
  }
}

#endif
