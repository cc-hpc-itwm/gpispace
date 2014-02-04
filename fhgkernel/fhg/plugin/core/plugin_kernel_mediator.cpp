#include <iostream>
#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/core/plugin_kernel_mediator.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <signal.h>

namespace fhg
{
  namespace core
  {
    PluginKernelMediator::PluginKernelMediator ( fhg::core::plugin_t::ptr_t const & p
                                               , kernel_t *k
                                               )
      : m_plugin(p)
      , _name (p->name())
      , m_kernel(k)
    {
      assert (m_plugin);
      assert (m_kernel);
    }

    fhg::core::plugin_t::ptr_t PluginKernelMediator::plugin()
    {
      return m_plugin;
    }

    std::string PluginKernelMediator::get(std::string const & key, std::string const &dflt) const
    {
      return m_kernel->get("plugin." + _name + "." + key, dflt);
    }

    int PluginKernelMediator::shutdown ()
    {
      DLOG(WARN, "plugin `" << _name << "' requested to stop the kernel!");
      m_kernel->stop();
      return 0;
    }

    int PluginKernelMediator::kill ()
    {
      LOG (WARN
          , "plugin `" << _name
          << "' requested to terminate the kernel!"
          );
      ::kill (SIGKILL, getpid ());
      return 0;
    }

    int PluginKernelMediator::terminate ()
    {
      LOG (WARN
          , "plugin `" << _name
          << "' requested to terminate the kernel!"
          );
      ::kill (SIGTERM, getpid ());
      return 0;
    }

    std::string const & PluginKernelMediator::get_name () const
    {
      return m_kernel->get_name ();
    }
  }
}
