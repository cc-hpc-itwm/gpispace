#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <boost/thread.hpp>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/plugin_kernel_mediator.hpp>

namespace fhg
{
  namespace core
  {
    class kernel_t
    {
    public:
      kernel_t ();
      ~kernel_t ();

      int load_plugin (std::string const & file);
      int unload_plugin (std::string const &name);

      void schedule_immediate(fhg::core::plugin_t::ptr_t, fhg::plugin::task_t);
      void schedule_later(fhg::core::plugin_t::ptr_t, fhg::plugin::task_t, unsigned long millis_from_now);
      plugin_t::ptr_t lookup_plugin(std::string const & name);
    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::shared_ptr<PluginKernelMediator> mediator_ptr;
      typedef std::map<std::string, mediator_ptr> plugin_map_t;

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;
      mutable mutex_type m_mtx_incomplete_plugins;

      /* loaded plugins, interdependencies, reference counts */
      plugin_map_t m_plugins;
      plugin_map_t m_incomplete_plugins;

      /* task queue */
      /* thread */
    };
  }
}

#endif
