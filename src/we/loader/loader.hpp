#ifndef WE_LOADER_MODULE_LOADER_HPP
#define WE_LOADER_MODULE_LOADER_HPP 1

#include <list>
#include <string>
#include <cassert>
#include <dlfcn.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility.hpp>

#include <we/loader/exceptions.hpp>
#include <we/loader/Module.hpp>

namespace we
{
  namespace loader
  {
    using boost::shared_ptr;

    class loader : boost::noncopyable
    {
    public:
      typedef shared_ptr<Module> module_ptr_t;
      typedef shared_ptr<loader> ptr_t;
      typedef std::list<boost::filesystem::path> search_path_t;
      typedef std::list<std::string> module_names_t;
      typedef boost::unordered_map<std::string, module_ptr_t> module_table_t;

      static ptr_t create();

      loader();
      ~loader();

      Module& operator[] (const std::string &module);

      module_ptr_t load (const std::string&, const boost::filesystem::path&);

      const search_path_t& search_path() const;
      void clear_search_path();
      void append_search_path (const boost::filesystem::path&);

    private:
      module_table_t module_table_;
      module_names_t module_load_order_;
      search_path_t search_path_;
      mutable boost::recursive_mutex mtx_;
    };
  }
}

#endif
