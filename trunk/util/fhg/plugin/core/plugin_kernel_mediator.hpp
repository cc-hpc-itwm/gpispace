#ifndef FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP
#define FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP 1

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
                           );

      ~PluginKernelMediator();

      virtual fhg::plugin::Plugin * acquire(std::string const & name);
      virtual void release(std::string const &name);

      void schedule(fhg::plugin::task_t);
      void schedule(fhg::plugin::task_t, size_t ticks);

      fhg::core::plugin_t::ptr_t plugin ();

      int start ();
      int stop ();

      std::string get(std::string const & key, std::string const &dflt) const;
      void start_completed(int);
    private:
      fhg::core::plugin_t::ptr_t m_plugin;
      fhg::core::kernel_t *m_kernel;
    };
  }
}

#endif
