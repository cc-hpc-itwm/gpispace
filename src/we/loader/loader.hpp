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

      static ptr_t create();

      ~loader();

      Module& operator[] (const std::string &module);

      module_ptr_t load (const std::string&, const boost::filesystem::path&);

      const search_path_t& search_path() const;
      void clear_search_path();
      void append_search_path (const boost::filesystem::path&);

    private:
      typedef boost::unordered_map<std::string, module_ptr_t> module_table_t;
      module_table_t module_table_;
      std::list<std::string> module_load_order_;
      search_path_t search_path_;
      mutable boost::recursive_mutex mtx_;
    };
  }
}

#endif
