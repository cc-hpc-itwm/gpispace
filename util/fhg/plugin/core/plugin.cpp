#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <fhglog/minimal.hpp>

#include <fhg/plugin/core/exception.hpp>
#include <fhg/plugin/magic.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

namespace fhg
{
  namespace core
  {
    plugin_t::plugin_t ( std::string const & my_name
                       , std::string const & my_filename
                       , const fhg_plugin_descriptor_t *my_desc
                       , int my_flags
                       , void *my_handle
                       )
      : m_name (my_name)
      , m_file_name(my_filename)
      , m_plugin (0)
      , m_descriptor (my_desc)
      , m_flags (my_flags)
      , m_handle (my_handle)
      , m_kernel (0)
      , m_started (false)
      , m_dependencies()
      , m_refcount(0)
    {
      assert (m_descriptor != 0);
    }

    plugin_t::~plugin_t ()
    {
      assert (! is_in_use());
      stop ();
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

    int plugin_t::init ()
    {
      char *error;
      fhg_plugin_create create_plugin;

      dlerror();
      *(void**)(&create_plugin) = dlsym(m_handle, "fhg_get_plugin_instance");
      if ((error = dlerror()) != NULL)
      {
        dlclose(m_handle);
        throw std::runtime_error("could not get create function: " + std::string(error));
      }

      m_plugin = create_plugin();
      dlerror();
      return 0;
    }

    int plugin_t::start (fhg::plugin::Kernel *kernel)
    {
      if (m_plugin)
      {
        m_kernel = kernel;
        m_started = true;
        return m_plugin->fhg_plugin_start_entry(m_kernel);
      }
      else
      {
        return -EINVAL;
      }
    }

    int plugin_t::stop ()
    {
      if   (is_in_use())
      {
        return -EBUSY;
      }
      else if (m_started)
      {
        int rc = m_plugin->fhg_plugin_stop_entry(m_kernel);

        if (rc == 0)
        {
          m_started = false;
          lock_type lock_dep (m_dependencies_mtx);
          while (! m_dependencies.empty())
          {
            ptr_t dep = m_dependencies.front(); m_dependencies.pop_front();
            dep->dec_refcount();
          }
        }

        return rc;
      }
      else
      {
        return 0;
      }
    }

    void plugin_t::handle_plugin_loaded (std::string const &name)
    {
      assert (m_plugin);
      assert (m_started);
      m_plugin->fhg_on_plugin_loaded (name);
    }

    void plugin_t::handle_plugin_unload (std::string const &name)
    {
      assert (m_plugin);
      assert (m_started);
      m_plugin->fhg_on_plugin_unload (name);
    }

    void plugin_t::handle_plugin_preunload (std::string const &name)
    {
      assert (m_plugin);
      assert (m_started);
      m_plugin->fhg_on_plugin_preunload (name);
    }

    plugin_t::ptr_t plugin_t::create (std::string const & filename, bool force)
    {
      return create(filename, force, RTLD_GLOBAL | RTLD_LAZY);
    }

    plugin_t::ptr_t plugin_t::create (std::string const & filename, bool force, int flags)
    {
      // dlopen file
      char *error;
      fhg_plugin_query query_plugin;
      void *handle;

      handle = dlopen(filename.c_str(), RTLD_GLOBAL | flags);
      if (!handle)
      {
        throw std::runtime_error("dlopen() failed: " + filename + dlerror());
      }

      dlerror();
      *(void**)(&query_plugin) = dlsym(handle, "fhg_query_plugin_descriptor");

      if ((error = dlerror()) != NULL)
      {
        dlclose(handle);
        throw std::runtime_error("could not get query function: " + std::string(error));
      }

      const fhg_plugin_descriptor_t *desc = query_plugin();
      if (desc == 0)
      {
        dlclose(handle);
        throw std::runtime_error("could not query plugin: no descriptor");
      }

      if (! force)
      {
        const std::string my_magic(FHG_PLUGIN_VERSION_MAGIC);
        const std::string plugin_magic (desc->magic);
        const std::string plugin_name (desc->name);

        if (my_magic != plugin_magic)
        {
          dlclose(handle);
          throw fhg::core::exception::plugin_version_magic_mismatch
            (plugin_name, plugin_magic, my_magic);
        }
      }

      return plugin_t::ptr_t( new plugin_t( desc->name
                                          , filename
                                          , desc
                                          , 0
                                          , handle
                                          )
                            );
    }
  }
}
