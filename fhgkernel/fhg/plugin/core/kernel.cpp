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
      : m_stop_requested (false)
      , m_running (false)
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

      // unload all non-static plugins according to dependency graph
      unload_all ();
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

    int kernel_t::load_plugin_by_name (std::string const & name)
    {
      lock_type load_plugin_lock (m_mtx_load_plugin);

      if (is_plugin_loaded (name))
        return 0;
      if (m_stop_requested)
        return ECANCELED;

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
    {
      lock_type load_plugin_lock (m_mtx_load_plugin);

      if (m_stop_requested)
        return ECANCELED;

      int rc = 0;

      bool load_lazy (boost::lexical_cast<bool>(get("kernel.load.lazy", "1")));

      std::string full_path_to_file = fs::absolute (fs::path (file)).string ();

      if (m_failed_path_cache.end () != std::find ( m_failed_path_cache.begin ()
                                                  , m_failed_path_cache.end ()
                                                  , full_path_to_file
                                                  )
         )
      {
        DMLOG ( TRACE
              , "avoiding another attempt to load from '"
              << full_path_to_file
              << "'"
              );
        return EFAULT;
      }

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
        fhg_plugin_query _fun;
      } query_plugin;

      query_plugin._ptr = dlsym(handle, "fhg_query_plugin_descriptor");

      if (char* error = dlerror())
      {
        dlclose(handle);
        throw std::runtime_error("could not get query function: " + std::string(error));
      }

      const fhg_plugin_descriptor_t *desc = query_plugin._fun();

      if (is_plugin_loaded (desc->name))
      {
        throw std::runtime_error
          ("another plugin with the same name is already loaded: " + std::string (desc->name));
      }

      std::list<plugin::Plugin*> dependencies;

      std::list<std::string> depends;
      fhg::util::split( desc->depends
                      , ","
                      , std::back_inserter (depends)
                      );

      BOOST_FOREACH(std::string const &dep, depends)
      {
        if (! lookup_plugin(dep))
        {
          if (0 != load_plugin_by_name (dep))
          {
            throw std::runtime_error("dependency not available: " + dep);
          }
        }
        dependencies.push_back (lookup_plugin (dep)->get_plugin());
      }

      plugin_t::ptr_t p (new plugin_t( desc->name
                                          , full_path_to_file
                                          , desc
                                          , handle
                                          )
                        );

      // create mediator
      // todo: write a control plugin that opens a socket or whatever
      mediator_ptr m
        (new PluginKernelMediator( p
                                 , this
                                 )
        );

      rc = p->init (m.get(), dependencies);
      if (rc == START_SUCCESSFUL) // started
      {
        {
          lock_type plugins_lock (m_mtx_plugins);
          m_plugins.insert (std::make_pair(p->name(), m));
          m_load_order.push_back (p->name ());
        }

        MLOG( TRACE
            , "loaded plugin '" << p->name() << "'"
            << " (from: '" << full_path_to_file << "')"
            );

        for ( plugin_map_t::iterator it (m_plugins.begin())
            ; it != m_plugins.end()
            ; ++it
            )
        {
          mediator_ptr other_mediator = it->second;
          if (it->first != p->name())
            other_mediator->plugin()->handle_plugin_loaded(p);
        }
      }
      else
      {
        m->stop();

        m_failed_path_cache.push_back (full_path_to_file);

        throw std::runtime_error
          ("plugin " + p->name() + " failed to start: " + std::string(strerror(-rc)));
      }

      return rc;
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

    int kernel_t::unload_plugin (kernel_t::plugin_map_t::iterator p)
    {
      assert (p != m_plugins.end());

      for ( plugin_map_t::iterator it (m_plugins.begin())
          ; it != m_plugins.end()
          ; ++it
          )
      {
        if (it->first != p->first)
        {
          mediator_ptr m = it->second;
          m->plugin()->handle_plugin_preunload(p->second->plugin());
        }
      }

      if (p->second->plugin()->is_in_use())
      {
        return -EBUSY;
      }

      int rc = p->second->stop();

      LOG(TRACE, "plugin '" << p->first << "' unloaded");

      m_load_order.remove (p->first);
      m_plugins.erase (p);

      return rc;
    }

    bool kernel_t::is_plugin_loaded (std::string const &name)
    {
      lock_type plugins_lock (m_mtx_plugins);
      return m_plugins.find(name) != m_plugins.end();
    }

    void kernel_t::unload_all ()
    {
      {
        lock_type plugins_lock (m_mtx_plugins);

        while (! m_plugins.empty())
        {
          std::string plugin_name = m_load_order.back ();

          plugin_names_t::reverse_iterator it = m_load_order.rbegin ();
          plugin_names_t::reverse_iterator end = m_load_order.rend ();

          for (; it != end ; ++it)
          {
            std::string plugin_to_unload = *it;
            plugin_map_t::iterator plugin_it =
              m_plugins.find (plugin_to_unload);
            assert (plugin_it != m_plugins.end ());

            if (unload_plugin (plugin_it) < 0)
              continue;
            else
              break;
          }
          /*
          for ( plugin_map_t::iterator it (m_plugins.begin())
              ; it != m_plugins.end()
              ; ++it
              )
          {
            if (unload_plugin (it) < 0)
              continue;
            else
              break;
          }
          */
        }
      }
    }

    void kernel_t::stop ()
    {
      m_stop_requested = true;
      _stop_request.notify();
    }

    int kernel_t::run()
    {
      assert (! m_running);

      m_running = true;
      m_failed_path_cache.clear ();

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
