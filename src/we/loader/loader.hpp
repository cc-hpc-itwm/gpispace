#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <we/loader/Module.hpp>
#include <we/loader/exceptions.hpp>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <list>
#include <string>
#include <stack>

namespace we
{
  namespace loader
  {
    using boost::shared_ptr;

    class loader : boost::noncopyable
    {
    public:
      typedef shared_ptr<loader> ptr_t;

      static ptr_t create();

      ~loader();

      Module& operator[] (const std::string &module);

      Module* load (const std::string&, const boost::filesystem::path&);

      const std::list<boost::filesystem::path>& search_path() const;
      void clear_search_path();
      void append_search_path (const boost::filesystem::path&);

    private:
      typedef boost::unordered_map<std::string, Module*> module_table_t;
      module_table_t module_table_;
      std::stack<Module*> module_load_order_;
      std::list<boost::filesystem::path> search_path_;
      mutable boost::recursive_mutex mtx_;
    };
  }
}

#endif
