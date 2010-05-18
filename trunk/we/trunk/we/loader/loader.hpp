#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <list>
#include <string>
#include <cassert>
#include <dlfcn.h>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

#include <we/loader/exceptions.hpp>
#include <we/loader/types.hpp>
#include <we/loader/Module.hpp>
#include <we/loader/module_traits.hpp>

namespace we {
  namespace loader {
    using boost::shared_ptr;

    class loader {
    public:
      typedef shared_ptr<Module> module_ptr_t;
      typedef shared_ptr<loader> ptr_t;
      typedef std::list<boost::filesystem::path> search_path_t;

      static ptr_t create() { return ptr_t(new loader()); }

      typedef boost::unordered_map<std::string, module_ptr_t> module_table_t;

      loader()
        : module_table_()
      {
        append_search_path (".");
      }

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
        boost::filesystem::path module_file_path;
        if (locate (module_name, module_file_path))
          return load (module_name, module_file_path);
        else
          throw ModuleLoadFailed("module '" + module_name + "' could not be located", module_name, "");
      }

      module_ptr_t load( const std::string & module_name
                       , const boost::filesystem::path & path
                       ) throw(ModuleException)
      {
        module_ptr_t mod(new Module(module_name, path.string()));
        std::pair<module_table_t::iterator, bool> insert_result =
          module_table_.insert(std::make_pair(mod->name(), mod));

        if (! insert_result.second)
        {
          throw ModuleLoadFailed("module already registered", mod->name(), path.string());
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

      void append_search_path (const boost::filesystem::path & p)
      {
        search_path_.push_back (p);
      }

      void prepend_search_path (const boost::filesystem::path & p)
      {
        search_path_.push_front (p);
      }

      void writeTo(std::ostream &os) const
      {
        os << "{loader, ";
        os << "{path, ";
        for (search_path_t::const_iterator p (search_path_.begin()); p != search_path_.end(); ++p)
        {
          if (p != search_path_.begin())
            os << ":";
          os << "\"" << *p << "\"";
        }
        os << "}";
        os << ", ";
        os << "{modules, ";

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
        os << "}";
        os << "}";
      }
    private:
      loader(const loader&);
      loader & operator = (const loader &);

      bool locate (const std::string & module, boost::filesystem::path & path_found)
      {
        namespace fs = boost::filesystem;
        const std::string file_name (module_traits<Module>::file_name (module));
        for (search_path_t::const_iterator dir (search_path_.begin()); dir != search_path_.end(); ++dir)
        {
          if (! fs::exists (*dir))
            continue;

          fs::path path (*dir / file_name);
          if (fs::exists (path))
          {
            path_found = path;
            return true;
          }
        }
        return false;
      }

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
      search_path_t search_path_;
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
