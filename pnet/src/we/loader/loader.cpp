#include "loader.hpp"

#include <fhglog/fhglog.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/show.hpp>

const int WE_GUARD_SYMBOL = 0xDEADBEEF;

namespace we {
  namespace loader {
    loader::ptr_t loader::create() { return ptr_t(new loader()); }

    loader::loader ()
      : module_table_()
      , module_counter_(0)
    {}

    loader::~loader()
    {
      try {
        unload_all();
      } catch (const std::exception &ex) {
      }
    }

    const loader::module_ptr_t loader::get (const std::string &module) const
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      module_table_t::const_iterator mod = module_table_.find(module);
      if (mod != module_table_.end()) {
        return mod->second;
      } else {
        throw ModuleNotLoaded(module);
      }
    }

    Module & loader::operator[] (const std::string &module)
    {
      return *get(module);
    }

    loader::module_ptr_t loader::get(const std::string &module)
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

    loader::module_ptr_t loader::load (const boost::filesystem::path & path)
    {
      return load ( "mod-"+fhg::util::show(module_counter_), path);
    }

    loader::module_ptr_t loader::load( const std::string & module_name
                                     , const boost::filesystem::path & path
                                     )
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
        MLOG (TRACE, "loaded module " << module_name << " from " << path);
        module_load_order_.push_front (module_name);
        ++module_counter_;
        return mod;
      }
    }

    void loader::unload(const std::string &module_name)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      module_table_t::iterator mod = module_table_.find(module_name);
      if (mod != module_table_.end()) {
        unload(mod);
      }
    }

    const loader::search_path_t & loader::search_path (void) const
    {
      return search_path_;
    }

    void loader::clear_search_path (void)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      search_path_.clear();
    }

    void loader::append_search_path (const boost::filesystem::path & p)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      search_path_.push_back (p);
    }

    void loader::prepend_search_path (const boost::filesystem::path & p)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      search_path_.push_front (p);
    }

    size_t loader::unload_autoloaded ()
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);

      size_t count (0);

      module_names_t::iterator it = module_load_order_.begin ();
      module_names_t::iterator end = module_load_order_.end ();
      while (it != end)
      {
        const std::string module_name = *it;
        if (module_name.find ("mod-") != 0)
        {
          it = module_load_order_.erase (it);
          unload (module_name);
          ++count;
        }
        else
        {
          ++it;
        }
      }
      return count;
    }

    int loader::selftest ()
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
          expr::eval::context inp;
          expr::eval::context out;
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

    static bool search_directory_for_file ( const boost::filesystem::path & path
                                          , const boost::filesystem::path & file
                                          , boost::filesystem::path & result
                                          )
    {
      namespace fs = boost::filesystem;

      try
      {
        fs::recursive_directory_iterator dir_iter =
          fs::recursive_directory_iterator (path, fs::symlink_option::recurse);
        const fs::recursive_directory_iterator end =
          fs::recursive_directory_iterator ();
        while (dir_iter != end)
        {
          const fs::path entry = *dir_iter;

          if (  fs::is_regular_file (entry)
             && entry.filename () == file
             )
          {
            result = entry;
            return true;
          }
          ++dir_iter;
        }
      }
      catch (fs::filesystem_error const &ex)
      {
        MLOG ( ERROR, "exception during directory scan in '"
             << path << "': " << ex.what ()
             );
        return false;
      }

      return false;
    }

    bool loader::locate (const std::string & module, boost::filesystem::path & path_found)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      namespace fs = boost::filesystem;
      const fs::path file_name = module_traits<Module>::file_name (module);
      for (search_path_t::const_iterator dir (search_path_.begin()); dir != search_path_.end(); ++dir)
      {
        if (search_directory_for_file (*dir, file_name, path_found))
          return true;
      }
      return false;
    }

    void loader::unload_all()
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      while (! module_load_order_.empty ())
      {
        const std::string module_name = module_load_order_.front ();
        module_load_order_.pop_front ();

        unload (module_name);
      }
    }

    loader::module_table_t::iterator loader::unload(module_table_t::iterator mod)
    {
      assert(mod != module_table_.end());
      MLOG (TRACE, "unloading " << mod->first);
      return module_table_.erase(mod);
    }
  }
}
