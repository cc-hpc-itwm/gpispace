#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <we/loader/Module.hpp>

#include <boost/filesystem.hpp>

#include <list>
#include <mutex>
#include <stack>
#include <string>
#include <unordered_map>

namespace we
{
  namespace loader
  {
    class loader : boost::noncopyable
    {
    public:
      loader() = default;
      loader (std::list<boost::filesystem::path> const&);
      ~loader();

      Module& operator[] (const std::string &module);

   private:
      std::mutex _table_mutex;
      typedef std::unordered_map<std::string, Module*> module_table_t;
      module_table_t _module_table;
      std::stack<Module*> _module_stack;
      std::list<boost::filesystem::path> const _search_path;
    };
  }
}

#endif
