#include "loader.hpp"

#include <fhg/assert.hpp>
#include <fhg/util/show.hpp>

#include <boost/foreach.hpp>

const int WE_GUARD_SYMBOL = 0xDEADBEEF;

namespace we
{
  namespace loader
  {
    loader::~loader()
    {
      while (not _module_stack.empty())
      {
        delete _module_stack.top();
        _module_stack.pop();
      }
    }

    Module& loader::operator[] (const std::string& module)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_loader_mutex);

      module_table_t::const_iterator mod (_module_table.find (module));

      if (mod != _module_table.end())
      {
        return *mod->second;
      }

      const boost::filesystem::path file_name ("lib" + module + ".so");

      BOOST_FOREACH (boost::filesystem::path const& p, _search_path)
      {
        if (boost::filesystem::exists (p / file_name))
        {
          return *load (module, p / file_name);
        }
      }

      throw ModuleLoadFailed
        ("module '" + module + "' could not be located", module, "[not-found]");
    }

    Module* loader::load ( const std::string& name
                         , const boost::filesystem::path& path
                         )
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_loader_mutex);

      if (_module_table.find (name) != _module_table.end())
      {
        throw ModuleLoadFailed
          ("module already registered", name, path.string());
      }

      Module* mod (new Module (name, path.string()));

      _module_stack.push (mod);

      return _module_table.insert (std::make_pair (name, mod)).first->second;
    }

    const std::list<boost::filesystem::path>& loader::search_path() const
    {
      return _search_path;
    }

    void loader::clear_search_path()
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_loader_mutex);

      _search_path.clear();
    }

    void loader::append_search_path (const boost::filesystem::path & p)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_loader_mutex);

      _search_path.push_back (p);
    }
  }
}
