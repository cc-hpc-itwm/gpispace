#include "loader.hpp"

#include <we/loader/exceptions.hpp>

#include <fhg/util/join.hpp>

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
      boost::unique_lock<boost::recursive_mutex> const _ (_table_mutex);

      module_table_t::const_iterator mod (_module_table.find (module));

      if (mod != _module_table.end())
      {
        return *mod->second;
      }

      const boost::filesystem::path file_name ("lib" + module + ".so");

      boost::unique_lock<boost::recursive_mutex> const __ (_search_path_mutex);

      BOOST_FOREACH (boost::filesystem::path const& p, _search_path)
      {
        if (boost::filesystem::exists (p / file_name))
        {
          return *load (module, p / file_name);
        }
      }

      throw module_not_found (file_name.string(), search_path());
    }

    Module* loader::load ( const std::string& name
                         , const boost::filesystem::path& path
                         )
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_table_mutex);

      if (_module_table.find (name) != _module_table.end())
      {
        throw module_already_registered (name);
      }

      Module* mod (new Module (path.string()));

      _module_stack.push (mod);

      return _module_table.insert (std::make_pair (name, mod)).first->second;
    }

    void loader::clear_search_path()
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_search_path_mutex);

      _search_path.clear();
    }

    void loader::append_search_path (const boost::filesystem::path & p)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_search_path_mutex);

      _search_path.push_back (p);
    }

    std::string loader::search_path() const
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_search_path_mutex);

      return fhg::util::join (_search_path.begin(), _search_path.end(), ":");
    }
  }
}
