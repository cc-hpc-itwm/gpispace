#include <iostream>
#include <fhglog/minimal.hpp>

#include <fhg/plugin/core/plugin_kernel_mediator.hpp>
#include <fhg/plugin/core/kernel.hpp>

namespace fhg
{
  namespace core
  {
    PluginKernelMediator::PluginKernelMediator ( fhg::core::plugin_t::ptr_t const & p
                                               , kernel_t *k
                                               , bool privileged
                                               )
      : m_plugin(p)
      , m_kernel(k)
      , m_storage (0)
      , m_privileged(privileged)
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

    fhg::plugin::Storage *PluginKernelMediator::storage()
    {
      if (! m_storage)
      {
        m_kernel->plugin_storage()->add_storage(m_plugin->name());
        m_storage = m_kernel->plugin_storage()->get_storage(m_plugin->name());
      }

      return m_storage;
    }

    void PluginKernelMediator::schedule(fhg::plugin::task_t task)
    {
      m_kernel->schedule (m_plugin->name(), task);
    }

    void PluginKernelMediator::schedule( fhg::plugin::task_t task
                                       , size_t ticks
                                       )
    {
      m_kernel->schedule (m_plugin->name(), task, ticks);
    }

    std::string PluginKernelMediator::get(std::string const & key, std::string const &dflt) const
    {
      return m_kernel->get("plugin." + m_plugin->name() + "." + key, dflt);
    }

    void PluginKernelMediator::start_completed(int ec)
    {
      m_kernel->plugin_start_completed(m_plugin->name(), ec);
    }

    int PluginKernelMediator::load_plugin (std::string const &path)
    {
      if (m_privileged)
      {
        try
        {
          return m_kernel->load_plugin(path);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not load plugin from " << path << ": " << ex.what());
          return -EINVAL;
        }
      }
      else
      {
        return -EPERM;
      }
    }

    int PluginKernelMediator::unload_plugin (std::string const &name)
    {
      if (m_privileged)
      {
        try
        {
          return m_kernel->unload_plugin(name);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not unload plugin " << name << ": " << ex.what());
          return -EINVAL;
        }
      }
      else
      {
        return -EPERM;
      }
    }
  }
}
