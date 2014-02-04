#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/plugin_kernel_mediator.hpp>

namespace fhg
{
  namespace core
  {
    class kernel_t
    {
    public:
      typedef std::vector<std::string> search_path_t;
      typedef std::list<std::string> plugin_names_t;

      kernel_t ( std::string const& name
               , fhg::core::kernel_t::search_path_t search_path
               );
      ~kernel_t ();

      int run();

      void stop ();

      int load_plugin (std::string const & entity);
      int load_plugin_by_name (std::string const & name);
      int load_plugin_from_file (std::string const & file);
      bool is_plugin_loaded (std::string const &name);

      void unload_all ();

      plugin_t::ptr_t lookup_plugin(std::string const & name);

      template <typename Iface>
      Iface* lookup_plugin_as (std::string const & name)
      {
        plugin_t::ptr_t plugin (this->lookup_plugin (name));
        return
          dynamic_cast<Iface*>(plugin->get_plugin ());
      }

      std::string get(std::string const & key, std::string const &dflt) const;
      std::string put(std::string const & key, std::string const &value);

      std::string const & get_name () const;
    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      typedef boost::shared_ptr<PluginKernelMediator> mediator_ptr;
      typedef std::map<std::string, std::pair<mediator_ptr, plugin_t::ptr_t> > plugin_map_t;
      typedef std::map<std::string, std::string> config_t;

      int unload_plugin (plugin_map_t::iterator it);

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;
      mutable mutex_type m_mtx_config;

      bool m_stop_requested;
      fhg::util::thread::event<> _stop_request;
      bool m_running;
      plugin_map_t   m_plugins;
      plugin_names_t m_load_order;

      config_t m_config;

      std::string m_name;
      search_path_t m_search_path;

      fhg::util::thread::event<> m_stopped;
    };
  }
}

#endif
