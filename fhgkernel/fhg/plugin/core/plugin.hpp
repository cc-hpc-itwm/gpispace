#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhg/plugin/descriptor.hpp>
#include <fhg/plugin/kernel.hpp>

namespace fhg
{
  namespace plugin
  {
    class Plugin;
  }

  namespace core
  {
    class plugin_t
    {
    public:
      typedef boost::shared_ptr<plugin_t> ptr_t;

      plugin_t ( std::string const & name
               , std::string const & filename
               , const fhg_plugin_descriptor_t *
               , void * handle
               , fhg::plugin::Kernel *kernel
               , std::list<plugin_t::ptr_t> deps
               );
      ~plugin_t ();

      bool is_in_use() const;

      void handle_plugin_loaded (plugin_t::ptr_t);
      void handle_plugin_preunload (plugin_t::ptr_t);
    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      void check_dependencies();

      void inc_refcount ();
      void dec_refcount ();

      mutable mutex_type m_refcount_mtx;
      mutable mutex_type m_dependencies_mtx;

      std::string m_name;
      std::string m_file_name;
      fhg::plugin::Plugin *m_plugin;
      const fhg_plugin_descriptor_t *m_descriptor;
      void *m_handle;

      std::list<ptr_t> m_dependencies;
      std::size_t m_refcount;
    };
  }
}

#endif
