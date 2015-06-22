#include <we/loader/loader.hpp>

#include <we/loader/exceptions.hpp>

#include <util-generic/join.hpp>
#include <util-generic/cxx14/make_unique.hpp>

const int WE_GUARD_SYMBOL = 0xDEADBEEF;

namespace we
{
  namespace loader
  {
    loader::loader (std::list<boost::filesystem::path> const& search_path)
      : _search_path (search_path)
    {}

    Module const& loader::operator[] (const std::string& module)
    {
      std::unique_lock<std::mutex> const _ (_table_mutex);

      std::unordered_map<std::string, std::unique_ptr<Module>>::const_iterator
        const mod (_module_table.find (module));

      if (mod != _module_table.end())
      {
        return *mod->second;
      }

      const boost::filesystem::path file_name ("lib" + module + ".so");

      for (boost::filesystem::path const& p : _search_path)
      {
        if (boost::filesystem::exists (p / file_name))
        {
          return *_module_table
            .emplace ( module
                     , fhg::util::cxx14::make_unique<Module> ((p / file_name).string())
                     )
            .first->second;
        }
      }

      throw module_not_found
        (file_name.string(), fhg::util::join (_search_path, ':').string());
    }
  }
}
