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

      explicit
      kernel_t (std::string const &state_path = "");
      kernel_t ( std::string const& state_path
               , std::string const& name
               , fhg::core::kernel_t::search_path_t search_path
               );
      ~kernel_t ();

      int run ();
      int run_and_unload (bool do_unload);

      void stop ();
      void wait_until_stopped ();

      void add_search_path (std::string const &);
      void clear_search_path ();
      void get_search_path (search_path_t &);

      int load_plugin (std::string const & entity);
      int load_plugin_by_name (std::string const & name);
      int load_plugin_from_file (std::string const & file);
      int unload_plugin (std::string const &name);
      bool is_plugin_loaded (std::string const &name);

      fhg::plugin::Storage * storage();
      fhg::plugin::Storage * plugin_storage();

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

      void set_name (std::string const &n);
      std::string const & get_name () const;
    private:
      void initialize_storage ();
      void require_dependencies (fhg::core::plugin_t::ptr_t const &);

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      typedef boost::shared_ptr<PluginKernelMediator> mediator_ptr;
      typedef std::map<std::string, mediator_ptr> plugin_map_t;
      typedef std::map<std::string, std::string> config_t;

      int unload_plugin (plugin_map_t::iterator it);

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;
      mutable mutex_type m_mtx_config;

      std::string m_state_path;
      bool m_stop_requested;
      fhg::util::thread::event<> _stop_request;
      bool m_running;
      plugin_map_t   m_plugins;
      plugin_names_t m_load_order;

      config_t m_config;

      mutable mutex_type m_mtx_storage;
      fhg::plugin::Storage *m_storage;

      std::string m_name;
      search_path_t m_search_path;
      search_path_t m_failed_path_cache;

      fhg::util::thread::event<> m_stopped;
    };
  }
}

#endif
