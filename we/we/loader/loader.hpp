#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <list>
#include <string>
#include <cassert>
#include <dlfcn.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

#include <we/loader/exceptions.hpp>
#include <we/loader/types.hpp>
#include <we/loader/Module.hpp>
#include <we/loader/module_traits.hpp>

#include <fhglog/fhglog.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/show.hpp>

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
        , module_counter_(0)
      {
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
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return mod->second;
        } else {
          throw ModuleNotLoaded(module);
        }
      }

      Module & operator[] (const std::string &module) throw(ModuleNotLoaded, ModuleLoadFailed)
      {
        return *get(module);
      }

      module_ptr_t get(const std::string &module) throw(ModuleNotLoaded, ModuleLoadFailed)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        module_table_t::const_iterator mod = module_table_.find(module);
        if (mod != module_table_.end()) {
          return mod->second;
        } else {
          boost::filesystem::path module_file_path;
          if (locate (module, module_file_path))
            return load (module, module_file_path);
          else
            throw ModuleLoadFailed("module '" + module + "' could not be located", module, "[not-found]");
        }
      }

      module_ptr_t load ( const boost::filesystem::path & path ) throw (ModuleLoadFailed)
      {
        return load ( "mod-"+fhg::util::show(module_counter_), path);
      }

      module_ptr_t load( const std::string & module_name
                       , const boost::filesystem::path & path
                       ) throw(ModuleException)
      {
        module_ptr_t mod(new Module(module_name, path.string()));

        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        std::pair<module_table_t::iterator, bool> insert_result =
          module_table_.insert(std::make_pair(module_name, mod));

        if (! insert_result.second)
        {
          throw ModuleLoadFailed("module already registered", module_name, path.string());
        }
        else
        {
          LOG(INFO, "loaded module " << module_name << " from " << path);
          ++module_counter_;
          return mod;
        }
      }

      void unload(const std::string &module_name)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        module_table_t::iterator mod = module_table_.find(module_name);
        if (mod != module_table_.end()) {
          unload(mod);
        }
      }

      const search_path_t & search_path (void) const
      {
        return search_path_;
      }

      void clear_search_path (void)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        search_path_.clear();
      }

      void append_search_path (const boost::filesystem::path & p)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        search_path_.push_back (p);
      }

      void prepend_search_path (const boost::filesystem::path & p)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        search_path_.push_front (p);
      }

      void writeTo(std::ostream &os) const
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);

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

      size_t unload_autoloaded ()
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);

        size_t count (0);
        module_table_t::iterator m(module_table_.begin());
        while (m != module_table_.end())
        {
          if (m->first.find("mod-") == 0)
          {
            ++m;
          }
          else
          {
            m = unload (m);
            ++count;
          }
        }
        return count;
      }

      int selftest ()
      {
        int ec (0);

        // running selftests
        for ( module_table_t::const_iterator m(module_table_.begin())
            ; m != module_table_.end()
            ; ++m
            )
        {
          try
          {
            we::loader::input_t inp;
            we::loader::output_t out;
            (*(m->second))("selftest", inp, out);
          }
          catch (FunctionNotFound const &)
          {
            // ignore
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "selftest failed for module: " << m->first << ": " << ex.what());
            ++ec;
          }
        }
        return ec;
      }
    private:
      loader(const loader&);
      loader & operator = (const loader &);

      bool locate (const std::string & module, boost::filesystem::path & path_found)
      {
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
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
        boost::unique_lock<boost::recursive_mutex> lock(mtx_);
        while (! module_table_.empty()) {
          unload(module_table_.begin());
        }
      }

      module_table_t::iterator unload(module_table_t::iterator mod)
      {
        assert(mod != module_table_.end());
        LOG(INFO, "unloading " << mod->first);
        return module_table_.erase(mod);
      }

      module_table_t module_table_;
      unsigned long module_counter_;
      search_path_t search_path_;
      mutable boost::recursive_mutex mtx_;
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
