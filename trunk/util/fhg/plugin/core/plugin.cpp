#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <stdexcept>
#include <algorithm>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

namespace fhg
{
  namespace core
  {
    plugin_t::plugin_t ( std::string const & my_name
                       , std::string const & my_filename
                       , fhg::plugin::Plugin *my_plugin
                       , const fhg_plugin_descriptor_t *my_desc
                       , int my_flags
                       , void *my_handle
                       )
      : m_name (my_name)
      , m_file_name(my_filename)
      , m_plugin (my_plugin)
      , m_descriptor (my_desc)
      , m_flags (my_flags)
      , m_handle (my_handle)
      , m_kernel (0)
      , m_started (false)
      , m_dependencies()
      , m_refcount(0)
    {
      assert (m_plugin != 0);
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

    size_t plugin_t::use_count () const
    {
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
      assert (other != 0);
      if (! is_depending_on(other))
      {
        m_dependencies.push_back(other);
        ++other->m_refcount;
      }
    }

    void plugin_t::del_dependency(const plugin_t::ptr_t &other)
    {
      if (is_depending_on(other))
      {
        m_dependencies.erase(std::find( m_dependencies.begin()
                                      , m_dependencies.end()
                                      , other
                                      )
                            );
        --other->m_refcount;
      }
    }

    int plugin_t::start (fhg::plugin::Kernel *kernel)
    {
      m_kernel = kernel;
      return m_plugin->fhg_plugin_start(m_kernel);
    }

    int plugin_t::stop ()
    {
      if   (is_in_use()) return -EBUSY;
      else               return m_plugin->fhg_plugin_stop(m_kernel);
    }

    plugin_t::ptr_t plugin_t::create (std::string const & filename, bool force)
    {
      // dlopen file
      char *error;
      fhg_plugin_query query_plugin;
      fhg_plugin_create create_plugin;
      void *handle;

      handle = dlopen(filename.c_str(), RTLD_LAZY | RTLD_GLOBAL);
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
        const std::string magic(FHG_PLUGIN_VERSION_MAGIC);
        if (magic != desc->magic)
        {
          dlclose(handle);
          throw std::runtime_error
            ("could not load plugin: version mismatch: expected := " + magic + " got := " + desc->magic);
        }
      }

      dlerror();
      *(void**)(&create_plugin) = dlsym(handle, "fhg_get_plugin_instance");
      if ((error = dlerror()) != NULL)
      {
        dlclose(handle);
        throw std::runtime_error("could not get create function: " + std::string(error));
      }

      fhg::plugin::Plugin* p = create_plugin();

      return plugin_t::ptr_t( new plugin_t( desc->name
                                          , filename
                                          , p
                                          , desc
                                          , 0
                                          , handle
                                          )
                            );
    }
  }
}
