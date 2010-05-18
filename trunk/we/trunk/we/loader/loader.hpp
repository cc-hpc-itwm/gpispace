#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <string>
#include <cassert>
#include <dlfcn.h>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <we/loader/exceptions.hpp>
#include <we/loader/types.hpp>
#include <we/loader/Module.hpp>

namespace we {
  namespace loader {
    using boost::shared_ptr;

    class loader {
    public:
      typedef shared_ptr<Module> module_ptr_t;
      typedef shared_ptr<loader> ptr_t;

      static ptr_t create() { return ptr_t(new loader()); }

      typedef boost::unordered_map<std::string, module_ptr_t> module_table_t;

      loader()
        : module_table_()
      {}

      ~loader()
      {
        try {
          unload_all();
        } catch (const std::exception &ex) {
        }
      }

      const module_ptr_t get(const std::string &module) const throw(ModuleNotLoaded)
      {
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return mod->second;
        } else {
          throw ModuleNotLoaded(module);
        }
      }

      Module & operator[] (const std::string &module) throw(ModuleNotLoaded)
      {
        return *get(module);
      }

      module_ptr_t get(const std::string &module) throw(ModuleNotLoaded)
      {
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return mod->second;
        } else {
          return load(module);
        }
      }

      module_ptr_t load ( const std::string & module_name ) throw (ModuleLoadFailed)
      {
        //    file_name = "lib" + module_name + ".so"
        // iterate over search path
        //    path = prefix + file_name
        return load (module_name, "./lib"+module_name+".so");
      }

      module_ptr_t load( const std::string & module_name
                         , const std::string & path
                         ) throw(ModuleException)
      {
        module_ptr_t mod(new Module(module_name, path));
        std::pair<module_table_t::iterator, bool> insert_result =
          module_table_.insert(std::make_pair(mod->name(), mod));

        if (! insert_result.second)
        {
          throw ModuleLoadFailed("module already registered", mod->name(), path);
        }
        else
        {
          return mod;
        }
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
      loader(const loader&);
      loader & operator = (const loader &);

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
                                    , const loader &l
                                    )
    {
      l.writeTo(os);
      return os;
    }
  }
}

#endif
