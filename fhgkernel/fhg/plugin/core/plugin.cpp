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
      , m_dependencies (deps)
      , m_refcount(0)
    {
      assert (m_descriptor != 0);

      dlerror();

      union
      {
        void* _ptr;
        fhg::plugin::Plugin* (*_fun)
          (fhg::plugin::Kernel*, std::list<fhg::plugin::Plugin*>);
      } create_plugin;

      create_plugin._ptr = dlsym(m_handle, "fhg_get_plugin_instance");

      if (char* error = dlerror())
      {
        dlclose(m_handle);
        throw std::runtime_error("could not get create function: " + std::string(error));
      }

      std::list<plugin::Plugin*> deps_raw;
      BOOST_FOREACH (ptr_t p, deps)
      {
        p->inc_refcount();
        deps_raw.push_back (p->m_plugin);
      }

      m_plugin = create_plugin._fun (kernel, deps_raw);
    }

    plugin_t::~plugin_t ()
    {
      assert (! is_in_use());

      delete m_plugin;
      m_plugin = NULL;

      lock_type lock_dep (m_dependencies_mtx);
      while (! m_dependencies.empty())
      {
        ptr_t dep = m_dependencies.front(); m_dependencies.pop_front();
        dep->dec_refcount();
      }

      if (m_handle) dlclose (m_handle);
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

    bool plugin_t::is_in_use () const
    {
      lock_type lock(m_refcount_mtx);
      return m_refcount > 0;
    }

    void plugin_t::handle_plugin_loaded (plugin_t::ptr_t other)
    {
      m_plugin->fhg_on_plugin_loaded (other->m_plugin);
    }

    void plugin_t::handle_plugin_preunload (plugin_t::ptr_t other)
    {
      m_plugin->fhg_on_plugin_preunload (other->m_plugin);
    }
  }
}
