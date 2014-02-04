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
    PluginKernelMediator::PluginKernelMediator (std::string name, kernel_t *k)
      : _name (name)
      , m_kernel(k)
    {
      assert (m_kernel);
    }

    std::string PluginKernelMediator::get(std::string const & key, std::string const &dflt) const
    {
      return m_kernel->get("plugin." + _name + "." + key, dflt);
    }

    void PluginKernelMediator::stop ()
    {
      DLOG(WARN, "plugin `" << _name << "' requested to stop the kernel!");
      m_kernel->stop();
    }

    std::string const & PluginKernelMediator::get_name () const
    {
      return m_kernel->get_name ();
    }
  }
}
