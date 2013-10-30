#ifndef FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP
#define FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP 1

#include <set>

#include <fhg/plugin/kernel.hpp>
#include <fhg/plugin/core/plugin.hpp>

namespace fhg
{
  namespace core
  {
    class kernel_t; // the actual kernel

    class PluginKernelMediator : public fhg::plugin::Kernel
    {
    public:
      PluginKernelMediator ( fhg::core::plugin_t::ptr_t const & plugin
                           , kernel_t *kernel
                           , bool privileged = false
                           );

      virtual fhg::plugin::Plugin * acquire(std::string const & name);
      virtual void release(std::string const &name);

      fhg::plugin::Storage *storage();

      size_t tick_time () const;
      void schedule(std::string const &, fhg::plugin::task_t);
      void schedule(std::string const &, fhg::plugin::task_t, size_t ticks);

      fhg::core::plugin_t::ptr_t plugin ();

      int start ();
      int stop ();

      std::string get(std::string const & key, std::string const &dflt) const;
      void start_completed(int);

      int load_plugin (std::string const &path);
      int unload_plugin (std::string const &name);

      int kill ();
      int terminate ();
      int shutdown ();

      std::string const & get_name () const;
    private:
      bool is_privileged () const;
      bool has_permission(int) const;

      fhg::core::plugin_t::ptr_t m_plugin;
      fhg::core::kernel_t *m_kernel;
      fhg::plugin::Storage *m_storage;
      std::set <int> m_permissions;
    };
  }
}

#endif
