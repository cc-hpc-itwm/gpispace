#include <unistd.h> // getuid, alarm, setsid, fork
#include <sys/types.h> // uid_t
#include <sys/time.h>
#include <dlfcn.h>
#include <cstring>
#include <algorithm>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/threadname.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#define START_SUCCESSFUL 0

namespace fhg
{
  namespace core
  {
    kernel_t::kernel_t ( std::string const& name
                       , fhg::core::kernel_t::search_path_t search_path
                       )
      : m_running (false)
      , m_name (name)
      , m_search_path (search_path)
    {
    }

    kernel_t::~kernel_t ()
    {
      if (m_running)
      {
        stop ();
        m_stopped.wait();
      }

      BOOST_REVERSE_FOREACH (std::string plugin_to_unload, m_load_order)
      {
        const plugin_map_t::iterator plugin (m_plugins.find (plugin_to_unload));

        BOOST_FOREACH (plugin_map_t::value_type other, m_plugins)
        {
          if (other.first != plugin->first)
          {
            other.second.second->handle_plugin_preunload (plugin->second.second);
          }
        }

        m_plugins.erase (plugin);

        LOG (TRACE, "plugin '" << plugin->first << "' unloaded");
      }
    }

    int kernel_t::load_plugin_by_name (std::string const & name)
    {
      lock_type load_plugin_lock (m_mtx_load_plugin);

      {
        lock_type plugins_lock (m_mtx_plugins);
        if (m_plugins.find(name) != m_plugins.end())
        {
          return 0;
        }
      }

      int ec = ENOENT;

      BOOST_FOREACH (std::string const &dir, m_search_path)
      {
        fs::path plugin_path (dir);
        plugin_path /= name + ".so";

        DMLOG (TRACE, "trying: " << plugin_path);

        if (fs::is_regular_file (plugin_path))
        {
          try
          {
            ec = load_plugin (plugin_path.string ());
            if (0 == ec)
              break;
          }
          catch (std::exception const &ex)
          {
            MLOG ( WARN
                 , "plugin '" << name
                 << "' could not be loaded from: " << plugin_path
                 << " : " << ex.what ()
                 );
            ec = EFAULT;
          }
        }
      }

      return ec;
    }

    int kernel_t::load_plugin_from_file (std::string const &file)
    try
    {
      lock_type load_plugin_lock (m_mtx_load_plugin);

      bool load_lazy (boost::lexical_cast<bool>(get("kernel.load.lazy", "1")));

      std::string full_path_to_file = fs::absolute (fs::path (file)).string ();

      if (fs::is_directory (full_path_to_file))
      {
        return EISDIR;
      }
      else if (not fs::is_regular_file (full_path_to_file))
      {
        return EINVAL;
      }


      // dlopen file
      void *handle (dlopen(full_path_to_file.c_str(), RTLD_GLOBAL | (load_lazy?RTLD_LAZY:RTLD_NOW)));
      if (!handle)
      {
        std::string msg ("dlopen () failed: ");
        msg += dlerror ();
        throw std::runtime_error(msg);
      }

      dlerror();

      union
      {
        void* _ptr;
        const fhg_plugin_descriptor_t* (*_fun)();
      } query_plugin;

      query_plugin._ptr = dlsym(handle, "fhg_query_plugin_descriptor");

      if (char* error = dlerror())
      {
        dlclose(handle);
        throw std::runtime_error("could not get query function: " + std::string(error));
      }

      const fhg_plugin_descriptor_t *desc = query_plugin._fun();
      const std::string plugin_name (desc->name);

      {
        lock_type plugins_lock (m_mtx_plugins);
        if (m_plugins.find (plugin_name) != m_plugins.end())
        {
          throw std::runtime_error
            ("another plugin with the same name is already loaded: " + plugin_name);
        }
      }

      std::list<plugin_t::ptr_t> dependencies;

      std::list<std::string> depends;
      fhg::util::split( desc->depends
                      , ","
                      , std::back_inserter (depends)
                      );

      {
        lock_type plugins_lock (m_mtx_plugins);

        BOOST_FOREACH(std::string const &dep, depends)
        {
          if (m_plugins.find (dep) == m_plugins.end())
          {
            if (0 != load_plugin_by_name (dep))
            {
              throw std::runtime_error("dependency not available: " + dep);
            }
          }

          dependencies.push_back (m_plugins.find (dep)->second.second);
        }
      }

      mediator_ptr m (new PluginKernelMediator (plugin_name, this));

      plugin_t::ptr_t p (new plugin_t( handle
                                     , m.get()
                                     , dependencies
                                          )
                        );

      {
        lock_type plugins_lock (m_mtx_plugins);
        m_plugins.insert (std::make_pair(plugin_name, std::make_pair (m, p)));
        m_load_order.push_back (plugin_name);
      }

      MLOG( TRACE
          , "loaded plugin '" << plugin_name << "'"
          << " (from: '" << full_path_to_file << "')"
          );

      for ( plugin_map_t::iterator it (m_plugins.begin())
          ; it != m_plugins.end()
          ; ++it
          )
      {
        if (it->first != plugin_name)
          it->second.second->handle_plugin_loaded(p);
      }

      return 0;
    }
    catch (std::runtime_error const& ex)
    {
      throw std::runtime_error
        ("plugin " + file + " failed to start: " + ex.what());
    }


    int kernel_t::load_plugin(std::string const & entity)
    {
      if (fs::exists (entity))
      {
        return load_plugin_from_file (entity);
      }
      else
      {
        return load_plugin_by_name (entity);
      }
    }

    void kernel_t::stop ()
    {
      _stop_request.notify();
    }

    int kernel_t::run()
    {
      assert (! m_running);

      m_running = true;

      _stop_request.wait();

      m_running = false;

      m_stopped.notify();

      return 0;
    }

    std::string kernel_t::get( std::string const & key
                             , std::string const &dflt
                             ) const
    {
      lock_type lock (m_mtx_config);
      config_t::const_iterator it (m_config.find(key));
      if (it == m_config.end()) return dflt;
      else                      return it->second;
    }

    std::string kernel_t::put( std::string const & key
                             , std::string const & val
                             )
    {
      lock_type lock (m_mtx_config);
      config_t::const_iterator it (m_config.find(key));
      std::string old;
      if (it != m_config.end())
      {
        old = it->second;
      }
      m_config[key] = val;
      return old;
    }

    std::string const & kernel_t::get_name () const
    {
      return m_name;
    }
  }
}
