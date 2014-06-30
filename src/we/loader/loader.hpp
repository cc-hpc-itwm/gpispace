#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <we/loader/Module.hpp>

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <list>
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
      loader (std::list<boost::filesystem::path>);
      ~loader();

      Module& operator[] (const std::string &module);

      Module* load (const std::string&, const boost::filesystem::path&);

      void clear_search_path();
      void append_search_path (const boost::filesystem::path&);

      std::string search_path() const;

   private:
      mutable boost::recursive_mutex _table_mutex;
      typedef std::unordered_map<std::string, Module*> module_table_t;
      module_table_t _module_table;
      std::stack<Module*> _module_stack;
      mutable boost::recursive_mutex _search_path_mutex;
      std::list<boost::filesystem::path> _search_path;
    };
  }
}

#endif
