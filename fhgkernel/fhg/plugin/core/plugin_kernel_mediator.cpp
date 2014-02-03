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
      , m_kernel(k)
    {
      assert (m_plugin);
      assert (m_kernel);
    }

    int PluginKernelMediator::stop ()
    {
      return m_plugin->stop();
    }

    fhg::plugin::Plugin * PluginKernelMediator::acquire(std::string const & name)
    {
      fhg::core::plugin_t::ptr_t p = m_kernel->lookup_plugin(name);
      if (p)
      {
        if (! p->is_depending_on(m_plugin))
        {
          m_plugin->add_dependency(p);
          return p->get_plugin();
        }
        else
        {
          throw std::runtime_error
            ("dependency cycle detected between " + name + " -> " + m_plugin->name());
        }
      }
      else
      {
        return 0;
      }
    }

    void PluginKernelMediator::release(std::string const &name)
    {
      fhg::core::plugin_t::ptr_t p = m_kernel->lookup_plugin(name);
      if (p)
      {
        m_plugin->del_dependency(p);
      }
    }

    fhg::core::plugin_t::ptr_t PluginKernelMediator::plugin()
    {
      return m_plugin;
    }

    std::string PluginKernelMediator::get(std::string const & key, std::string const &dflt) const
    {
      return m_kernel->get("plugin." + m_plugin->name() + "." + key, dflt);
    }

    int PluginKernelMediator::shutdown ()
    {
      DLOG(WARN, "plugin `" << m_plugin->name() << "' requested to stop the kernel!");
      m_kernel->stop();
      return 0;
    }

    int PluginKernelMediator::kill ()
    {
      LOG (WARN
          , "plugin `" << m_plugin->name()
          << "' requested to terminate the kernel!"
          );
      ::kill (SIGKILL, getpid ());
      return 0;
    }

    int PluginKernelMediator::terminate ()
    {
      LOG (WARN
          , "plugin `" << m_plugin->name()
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
