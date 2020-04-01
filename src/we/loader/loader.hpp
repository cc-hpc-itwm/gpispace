#pragma once

#include <we/loader/Module.hpp>

#include <boost/filesystem.hpp>

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace we
{
  namespace loader
  {
    class loader : boost::noncopyable
    {
    public:
      loader (std::list<boost::filesystem::path> const&);

      Module const& operator[] (const std::string &m)
      {
        return module (false, m);
      }
      Module const& module
        ( bool require_module_unloads_without_rest
        , const std::string &module
        );

   private:
      std::mutex _table_mutex;
      std::unordered_map<std::string, std::unique_ptr<Module>> _module_table;
      std::list<boost::filesystem::path> const _search_path;
    };
  }
}
