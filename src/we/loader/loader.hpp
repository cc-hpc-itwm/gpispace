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

#include <we/loader/exceptions.hpp>
#include <we/loader/Module.hpp>

namespace we {
  namespace loader {
    using boost::shared_ptr;

    class loader {
    public:
      typedef shared_ptr<Module> module_ptr_t;
      typedef shared_ptr<loader> ptr_t;
      typedef std::list<boost::filesystem::path> search_path_t;
      typedef std::list<std::string> module_names_t;

      static ptr_t create();

      typedef boost::unordered_map<std::string, module_ptr_t> module_table_t;

      loader();
      ~loader();

      Module & operator[] (const std::string &module);

      module_ptr_t load (const boost::filesystem::path & path);

      module_ptr_t load( const std::string & module_name
                       , const boost::filesystem::path & path
                       );

      void unload(const std::string &module_name);

      const search_path_t & search_path (void) const;

      void clear_search_path (void);

      void append_search_path (const boost::filesystem::path & p);

      size_t unload_autoloaded ();
    private:
      loader(const loader&);
      loader & operator = (const loader &);

      module_table_t module_table_;
      module_names_t module_load_order_;
      unsigned long module_counter_;
      search_path_t search_path_;
      mutable boost::recursive_mutex mtx_;
    };
  }
}

#endif
