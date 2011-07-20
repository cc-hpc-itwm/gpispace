#include <fhg/plugin/config.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <boost/thread.hpp>

#define START_SUCCESSFUL 0
#define START_INCOMPLETE 1

namespace fhg
{
  namespace core
  {
    kernel_t::kernel_t ()
    {}

    kernel_t::~kernel_t ()
    {
      // unload all non-static plugins according to dependency graph
    }

    plugin_t::ptr_t kernel_t::lookup_plugin(std::string const &name)
    {
      lock_type plugins_lock (m_mtx_plugins);
      plugin_map_t::iterator p (m_plugins.find(name));
      if (p != m_plugins.end())
      {
        return p->second->plugin();
      }
      else
      {
        return plugin_t::ptr_t();
      }
    }

    int kernel_t::load_plugin(std::string const & file)
    {
      lock_type load_plugin_lock (m_mtx_load_plugin);
      int rc = 0;

      try
      {
        plugin_t::ptr_t p (plugin_t::create (file, false));
        {
          lock_type plugins_lock (m_mtx_plugins);
          if (m_plugins.find(p->name()) != m_plugins.end())
          {
            // TODO: print descriptor of other plugin
            throw std::runtime_error
              ("another plugin with the same name is already loaded: " + p->name());
          }
        }

        // create mediator
        mediator_ptr m(new PluginKernelMediator(p, this));

        // check_dependencies(p);
        rc = p->start (m.get());
        if (rc == START_SUCCESSFUL) // started
        {
          lock_type plugins_lock (m_mtx_plugins);
          m_plugins.insert (std::make_pair(p->name(), m));

          // notify loaded plugins about the new plugin
          for ( plugin_map_t::iterator it (m_plugins.begin())
              ; it != m_plugins.end()
              ; ++it
              )
          {
            it->second->plugin_loaded (p->name());
          }
        }
        else if (rc == START_INCOMPLETE) // incomplete
        {
          lock_type plugins_lock (m_mtx_incomplete_plugins);
          m_incomplete_plugins.insert (std::make_pair(p->name(), m));
        }
        else
        {
          throw std::runtime_error
            ("plugin " + p->name() + " failed to start!");
        }
      }
      catch (std::exception const & ex)
      {
        throw;
      }

      return rc;
    }

    int kernel_t::unload_plugin (std::string const &name)
    {
      plugin_t::ptr_t plugin = lookup_plugin(name);
      if (plugin)
      {
        if (plugin->is_in_use())
        {
          return -EBUSY;
        }
        else
        {
          return 0;
        }
      }
      else
      {
        return -ESRCH;
      }
    }
  }
}
