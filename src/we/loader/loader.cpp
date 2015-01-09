#include <we/loader/loader.hpp>

#include <we/loader/exceptions.hpp>

#include <fhg/util/join.hpp>

const int WE_GUARD_SYMBOL = 0xDEADBEEF;

namespace we
{
  namespace loader
  {
    loader::loader (std::list<boost::filesystem::path> search_path)
      : _search_path (search_path)
    {}

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
      std::unique_lock<std::recursive_mutex> const _ (_table_mutex);

      module_table_t::const_iterator mod (_module_table.find (module));

      if (mod != _module_table.end())
      {
        return *mod->second;
      }

      const boost::filesystem::path file_name ("lib" + module + ".so");

      for (boost::filesystem::path const& p : _search_path)
      {
        if (boost::filesystem::exists (p / file_name))
        {
          Module* mod (new Module ((p / file_name).string()));

          _module_stack.push (mod);

          return *_module_table.emplace (module, mod).first->second;
        }
      }

      throw module_not_found
        ( file_name.string()
        , fhg::util::join (_search_path.begin(), _search_path.end(), ":")
        );
    }
  }
}
