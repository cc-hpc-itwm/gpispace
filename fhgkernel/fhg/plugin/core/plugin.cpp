#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/core/exception.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace core
  {
    plugin_t::plugin_t ( std::string const & my_name
                       , std::string const & my_filename
                       , const fhg_plugin_descriptor_t *my_desc
                       , void *my_handle
                       , fhg::plugin::Kernel *kernel
                       , std::list<plugin_t::ptr_t> deps
                       )
      : m_name (my_name)
      , m_file_name(my_filename)
      , m_plugin (0)
      , m_descriptor (my_desc)
      , m_handle (my_handle)
      , m_started (false)
      , m_dependencies (deps)
      , m_refcount(0)
    {
      assert (m_descriptor != 0);

      dlerror();

      union
      {
        void* _ptr;
        fhg_plugin_create _fun;
      } create_plugin;

      create_plugin._ptr = dlsym(m_handle, "fhg_get_plugin_instance");

      if (char* error = dlerror())
      {
        dlclose(m_handle);
        throw std::runtime_error("could not get create function: " + std::string(error));
      }

      m_plugin = create_plugin._fun();

      m_started = true;

      std::list<plugin::Plugin*> deps_raw;
      BOOST_FOREACH (ptr_t p, deps)
      {
        deps_raw.push_back (p->get_plugin());
      }
      m_plugin->fhg_plugin_start_entry(kernel, deps_raw);
    }

    plugin_t::~plugin_t ()
    {
      assert (! is_in_use());

      if (m_started)
      {
        m_plugin->fhg_plugin_stop();

        m_started = false;
        lock_type lock_dep (m_dependencies_mtx);
        while (! m_dependencies.empty())
        {
          ptr_t dep = m_dependencies.front(); m_dependencies.pop_front();
          dep->dec_refcount();
        }
      }

      if (m_plugin) delete m_plugin;
      if (m_handle) dlclose (m_handle);
    }

    std::string const & plugin_t::name () const
    {
      return m_name;
    }

    const fhg_plugin_descriptor_t *plugin_t::descriptor() const
    {
      return m_descriptor;
    }

    fhg::plugin::Plugin *plugin_t::get_plugin()
    {
      return m_plugin;
    }

    const fhg::plugin::Plugin *plugin_t::get_plugin() const
    {
      return m_plugin;
    }

    void plugin_t::inc_refcount ()
    {
      lock_type lock(m_refcount_mtx);
      ++m_refcount;
    }

    void plugin_t::dec_refcount ()
    {
      lock_type lock(m_refcount_mtx);
      assert (m_refcount > 0);
      --m_refcount;
    }

    size_t plugin_t::use_count () const
    {
      lock_type lock(m_refcount_mtx);
      return m_refcount;
    }

    bool plugin_t::is_depending_on(const plugin_t::ptr_t &other) const
    {
      assert (other != 0);

      return std::find( m_dependencies.begin()
                      , m_dependencies.end()
                      , other
                      ) != m_dependencies.end();
    }

    void plugin_t::add_dependency(const plugin_t::ptr_t &other)
    {
      lock_type lock(m_dependencies_mtx);

      assert (other != 0);

      if (! is_depending_on(other))
      {
        m_dependencies.push_back(other);
        other->inc_refcount();
      }
    }

    void plugin_t::del_dependency(const plugin_t::ptr_t &other)
    {
      lock_type lock(m_dependencies_mtx);

      if (is_depending_on(other))
      {
        m_dependencies.erase(std::find( m_dependencies.begin()
                                      , m_dependencies.end()
                                      , other
                                      )
                            );
        other->dec_refcount();
      }
    }

    void plugin_t::handle_plugin_loaded (plugin_t::ptr_t other)
    {
      assert (m_plugin);
      assert (m_started);
      m_plugin->fhg_on_plugin_loaded (other->get_plugin());
    }

    void plugin_t::handle_plugin_preunload (plugin_t::ptr_t other)
    {
      assert (m_plugin);
      assert (m_started);
      m_plugin->fhg_on_plugin_preunload (other->get_plugin());
    }
  }
}
