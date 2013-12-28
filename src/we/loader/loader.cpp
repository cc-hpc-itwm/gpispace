#include "loader.hpp"

#include <fhglog/fhglog.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/show.hpp>

#include <boost/foreach.hpp>

const int WE_GUARD_SYMBOL = 0xDEADBEEF;

namespace we {
  namespace loader {
    loader::ptr_t loader::create() { return ptr_t(new loader()); }

    loader::loader ()
      : module_table_()
    {}

    loader::~loader()
    {
      while (! module_load_order_.empty())
      {
        const std::string module_name (module_load_order_.front());
        module_load_order_.pop_front();

        module_table_t::iterator const mod (module_table_.find (module_name));
        if (mod != module_table_.end())
        {
          MLOG (TRACE, "unloading " << mod->first);
          module_table_.erase (mod);
        }
      }
    }

    Module& loader::operator[] (const std::string& module)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (mtx_);

      module_table_t::const_iterator mod (module_table_.find (module));

      if (mod != module_table_.end())
      {
        return *mod->second;
      }

      const boost::filesystem::path file_name ("lib" + module + ".so");

      BOOST_FOREACH (boost::filesystem::path const& p, search_path_)
      {
        if (boost::filesystem::exists (p / file_name))
        {
          return *load (module, p / file_name);
        }
      }

      throw ModuleLoadFailed
        ("module '" + module + "' could not be located", module, "[not-found]");
    }

    loader::module_ptr_t loader::load ( const std::string& name
                                      , const boost::filesystem::path& path
                                      )
    {
      boost::unique_lock<boost::recursive_mutex> const _ (mtx_);

      if (module_table_.find (name) != module_table_.end())
      {
        throw ModuleLoadFailed
          ("module already registered", name, path.string());
      }

      module_ptr_t const mod (new Module (name, path.string()));

      module_table_.insert (std::make_pair (name, mod));

      module_load_order_.push_front (name);

      MLOG (TRACE, "loaded module " << name << " from " << path);

      return mod;
    }

    void loader::unload(const std::string &module_name)
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      module_table_t::iterator mod = module_table_.find(module_name);
      if (mod != module_table_.end()) {
        MLOG (TRACE, "unloading " << mod->first);
        module_table_.erase (mod);
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
  }
}
