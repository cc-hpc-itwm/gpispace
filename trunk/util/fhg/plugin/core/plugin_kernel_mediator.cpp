#include <iostream>

#include <fhg/plugin/core/plugin_kernel_mediator.hpp>
#include <fhg/plugin/core/kernel.hpp>

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

    int PluginKernelMediator::start ()
    {
      return m_plugin->start(this);
    }

    int PluginKernelMediator::stop ()
    {
      return m_plugin->stop();
    }

    PluginKernelMediator::~PluginKernelMediator()
    {}

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

    void PluginKernelMediator::schedule_immediate(fhg::plugin::task_t task)
    {
      m_kernel->schedule_immediate (m_plugin->name(), task);
    }

    void PluginKernelMediator::schedule_later( fhg::plugin::task_t task
                                             , size_t ticks
                                             )
    {
      m_kernel->schedule_later (m_plugin->name(), task, ticks);
    }
  }
}
