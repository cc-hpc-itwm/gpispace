#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

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

      Module const& operator[] (const std::string &module);

   private:
      std::mutex _table_mutex;
      std::unordered_map<std::string, std::unique_ptr<Module>> _module_table;
      std::list<boost::filesystem::path> const _search_path;
    };
  }
}

#endif
