#include <unistd.h> // getuid, alarm, setsid, fork
#include <sys/types.h> // uid_t
#include <sys/time.h>
#include <dlfcn.h>
#include <cstring>
#include <algorithm>

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/config.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/core/null_storage.hpp>
#include <fhg/plugin/core/file_storage.hpp>

#include <fhg/util/split.hpp>
#include <fhg/util/threadname.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#define START_SUCCESSFUL 0
#define START_INCOMPLETE 1

void fhgRegisterStaticPlugin(fhg_plugin_query q, fhg_plugin_create c)
{
  LOG(ERROR, "TODO: register static plugin: " << q()->name);
}

namespace fhg
{
  namespace core
  {
    kernel_t::kernel_t (std::string const &state_path)
      : m_state_path (state_path)
      , m_tick_time (5 * 100 * 1000)
      , m_stop_requested (false)
      , m_running (false)
      , m_storage (0)
    {
      initialize_storage ();
    }

    kernel_t::~kernel_t ()
    {
      // unload all non-static plugins according to dependency graph
      unload_all ();

      // warning:  we must  not have  any  tasks stored,  since we  would get  a
      // segfault due to dynamic symbols
      assert (m_task_queue.empty());
      assert (m_pending_tasks.empty());

      if (m_storage)
      {
        delete m_storage;
        m_storage = 0;
      }
    }

    void kernel_t::add_search_path (std::string const &path)
    {
      m_search_path.push_back (path);
    }

    void kernel_t::clear_search_path ()
    {
      m_search_path.clear ();
    }

    void kernel_t::get_search_path (search_path_t & search_path)
    {
      m_search_path.insert ( search_path.end ()
                           , m_search_path.begin ()
                           , m_search_path.end ()
                           );
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

      bool load_force (boost::lexical_cast<bool>(get("kernel.load.force", "0")));
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

      plugin_t::ptr_t p (plugin_t::create( full_path_to_file
                                         , load_force
                                         , (load_lazy?RTLD_LAZY:RTLD_NOW)
                                         )
                        );
      {
        if (is_plugin_loaded (p->name ()))
        {
          throw std::runtime_error
            ("another plugin with the same name is already loaded: " + p->name());
        }
      }

      require_dependencies (p);

      // create mediator
      // todo: privileged plugin decision...
      // todo: write a control plugin that opens a socket or whatever
      mediator_ptr m
        (new PluginKernelMediator( p
                                 , this
                                 , true // TODO: "control" == p->name()
                                 )
        );

      rc = p->init();
      if (rc)
      {
        throw std::runtime_error("something went wrong during plugin initialization");
      }

      rc = m->start ();
      if (rc == START_SUCCESSFUL) // started
      {
        {
          lock_type plugins_lock (m_mtx_plugins);
          m_plugins.insert (std::make_pair(p->name(), m));
          m_load_order.push_back (p->name ());
        }

        MLOG( TRACE
            , p->name() << " plugin loaded "
            << "(from: " << full_path_to_file << ")"
            );

        for ( plugin_map_t::iterator it (m_plugins.begin())
            ; it != m_plugins.end()
            ; ++it
            )
        {
          mediator_ptr other_mediator = it->second;
          if (it->first != p->name())
            other_mediator->plugin()->handle_plugin_loaded(p->name());
        }
      }
      else if (rc == START_INCOMPLETE) // incomplete
      {
        lock_type plugins_lock (m_mtx_incomplete_plugins);
        m_incomplete_plugins.insert (std::make_pair(p->name(), m));
        m_load_order.push_back (p->name ());

        MLOG(TRACE, "start of " << p->name() << " incomplete");
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
          m->plugin()->handle_plugin_preunload(p->first);
        }
      }

      if (p->second->plugin()->is_in_use())
      {
        return -EBUSY;
      }

      remove_pending_tasks(p->first);

      int rc = p->second->stop();

      for ( plugin_map_t::iterator it (m_plugins.begin())
          ; it != m_plugins.end()
          ; ++it
          )
      {
        if (it->first != p->first)
        {
          it->second->plugin()->handle_plugin_unload (p->first);
        }
      }

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

    int kernel_t::handle_signal (int signum, siginfo_t *info, void *ctxt)
    {
      plugin_map_t to_signal;
      {
        if (not m_mtx_plugins.try_lock ())
        {
          DMLOG (WARN, "ignoring signal: mutex still locked");
          errno = EAGAIN;
          return -1;
        }
        to_signal = m_plugins;

        m_mtx_plugins.unlock ();
      }

      DMLOG (DEBUG, "handling signal: " << signum);

      for ( plugin_map_t::iterator it = to_signal.begin ()
          ; it != to_signal.end()
          ; ++it
          )
      {
        it->second->plugin()->handle_plugin_signal ( signum
                                                   , info
                                                   , ctxt
                                                   );
      }

      return 0;
    }

    static bool is_owner_of_task ( const std::string & p
                                 , const task_info_t & info
                                 )
    {
      return p == info.owner;
    }

    void kernel_t::remove_pending_tasks(std::string const &owner)
    {
      lock_type lock_pending (m_mtx_pending_tasks);
      task_queue_t::lock_type lock_task_q (m_task_queue.get_mutex());

      m_task_queue.remove_if
        (boost::bind (&is_owner_of_task, owner, _1));

      for ( task_list_t::iterator it = m_pending_tasks.begin()
          ; it != m_pending_tasks.end()
          ; // done explicitly
          )
      {
        if (it->owner == owner)
        {
          it = m_pending_tasks.erase(it);
        }
        else
        {
          ++it;
        }
      }
    }

    int kernel_t::unload_plugin (std::string const &name)
    {
      plugin_map_t::iterator it = m_plugins.find(name);
      if (it != m_plugins.end())
      {
        return unload_plugin(it);
      }
      else
      {
        return -ESRCH;
      }
    }

    void kernel_t::plugin_start_completed(std::string const & name, int ec)
    {
      lock_type plugins_lock (m_mtx_incomplete_plugins);

      plugin_map_t::iterator p_it (m_incomplete_plugins.find(name));
      mediator_ptr m = p_it->second;
      if (p_it == m_incomplete_plugins.end())
      {
        MLOG(WARN, "got completion event for illegal plugin: " << name);
        return;
      }
      else
      {
        m_incomplete_plugins.erase (p_it);
      }

      if (ec)
      {
        remove_pending_tasks(name);
        m->stop();
      }
      else
      {
        lock_type plugins_lock (m_mtx_plugins);
        m_plugins.insert (std::make_pair(name, m));

        MLOG(TRACE, name << " plugin finished starting");

        schedule( "kernel"
                , "notify plugin load"
                , boost::bind( &kernel_t::notify_plugin_load
                             , this
                             , name
                             )
                );
      }
    }

    void kernel_t::plugin_failed (std::string const &name, int ec)
    {
      LOG(ERROR, "plugin " << name << " failed with error-code: " << ec);
      // inform all dependent plugins that a plugin failed
      // this might be recursive
      //    unload affected plugin
      // we need an interface to tell the kernel to load/unload a plugin
    }

    void kernel_t::notify_plugin_load (std::string const & name)
    {
      // inform regular plugins
      DMLOG(TRACE, "notifying plugins that " << name << " is now available...");
      {
        lock_type plugins_lock (m_mtx_plugins);
        for ( plugin_map_t::iterator it (m_plugins.begin())
            ; it != m_plugins.end()
            ; ++it
            )
        {
          it->second->plugin()->handle_plugin_loaded(name);
        }
      }

      DMLOG(TRACE, "informing incomplete plugins that " << name << " is available ...");

      // inform incomplete plugins
      {
        std::vector<mediator_ptr> to_inform;

        {
          lock_type lock (m_mtx_incomplete_plugins);
          for ( plugin_map_t::iterator it (m_incomplete_plugins.begin())
              ; it != m_incomplete_plugins.end()
              ; ++it
              )
          {
            to_inform.push_back(it->second);
          }
        }

        for ( std::vector<mediator_ptr>::iterator it(to_inform.begin())
            ; it != to_inform.end()
            ; ++it
            )
        {
          DMLOG(TRACE, "notifying " << (*it)->plugin()->name());
          (*it)->plugin()->handle_plugin_loaded(name);
        }
      }
    }

    void kernel_t::require_dependencies (plugin_t::ptr_t const &plugin)
    {
      std::list<std::string> depends;
      fhg::util::split( plugin->descriptor()->depends
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
      }
    }

    void kernel_t::unload_all ()
    {
      {
        lock_type plugins_lock (m_mtx_incomplete_plugins);
        while (! m_incomplete_plugins.empty())
        {
          mediator_ptr m = m_incomplete_plugins.begin()->second;
          m_incomplete_plugins.erase(m_incomplete_plugins.begin());
          remove_pending_tasks (m->plugin()->name());
        }
      }

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

    void kernel_t::schedule( std::string const &owner
                           , std::string const &name
                           , fhg::plugin::task_t task
                           )
    {
      schedule (owner, name, task, 0);
    }

    void kernel_t::schedule( std::string const &owner
                           , std::string const &name
                           , fhg::plugin::task_t task
                           , size_t ticks
                           )
    {
      lock_type lock(m_mtx_pending_tasks);
      m_pending_tasks.push_back(task_info_t(owner, name, task, ticks));
    }

    void kernel_t::stop ()
    {
      m_stop_requested = true;
      if (! m_running)
      {
        return;
      }
    }

    void kernel_t::reset ()
    {
      m_stop_requested = false;
    }

    fhg::plugin::Storage* kernel_t::storage ()
    {
      return m_storage;
    }

    fhg::plugin::Storage* kernel_t::plugin_storage ()
    {
      return storage()->get_storage("plugin");
    }

    void kernel_t::initialize_storage()
    {
      assert (0 == m_storage);

      if (! m_state_path.empty())
      {
        try
        {
          m_storage = new fhg::plugin::core::FileStorage ( m_state_path
                                                         , O_CREAT
                                                         );
        }
        catch (std::exception const &ex)
        {
          MLOG(ERROR, "could not create file storage: " << ex.what());
          MLOG(WARN, "falling back to null-storage, persistence layer is not available!");
        }
      }

      if (0 == m_storage)
      {
        m_storage = new fhg::plugin::core::NullStorage;
      }

      int ec = m_storage->add_storage("plugin");
      if (0 != ec)
      {
        delete m_storage;
        MLOG(ERROR, "could not create 'plugin' storage area: " << strerror(ec));
        MLOG(WARN, "falling back to null-storage, persistence layer is not available!");
        m_storage = new fhg::plugin::core::NullStorage;
      }
    }

    int kernel_t::run ()
    {
      return this->run_and_unload (true);
    }

    int kernel_t::run_and_unload (bool unload_at_end)
    {
      struct timeval tv_start;
      struct timeval tv_diff;
      struct timeval tv_end;

      assert (! m_running);

      m_running = true;
      m_failed_path_cache.clear ();
      m_task_handler = boost::thread (&kernel_t::task_handler, this);
      fhg::util::set_threadname (m_task_handler, "[kernel-tasks]");

      const bool daemonize
        (boost::lexical_cast<bool>(get("kernel.daemonize", "0")));
      if (daemonize)
      {
        if (0 == fork())
        {
          setsid();
        }
        else
        {
          _exit (0);
        }
      }

      while (!m_stop_requested)
      {
        gettimeofday(&tv_start, 0);

        { /* decrement tick count of pending tasks and move them */
          lock_type lock_pending (m_mtx_pending_tasks);
          lock_type lock_task_q (m_mtx_task_queue);
          for ( task_list_t::iterator it = m_pending_tasks.begin()
              ; it != m_pending_tasks.end()
              ;
              )
          {
            if (0 == it->ticks)
            {
              m_task_queue.put (*it);
              it = m_pending_tasks.erase (it);
            }
            else
            {
              --it->ticks;
              ++it;
            }
          }
        }

        gettimeofday(&tv_end, 0);
        timersub (&tv_end, &tv_start, &tv_diff);
        if (tv_diff.tv_usec < m_tick_time)
        {
          usleep (m_tick_time - tv_diff.tv_usec);
        }
      }

      m_task_handler.interrupt();
      m_task_handler.join();

      if (unload_at_end)
      {
        unload_all ();
      }

      m_running = false;

      return 0;
    }

    time_t kernel_t::tick_time () const
    {
      return m_tick_time;
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

    void kernel_t::task_handler ()
    {
      while (!m_stop_requested)
      {
        task_info_t task = m_task_queue.get();
        try
        {
          DMLOG(TRACE, "executing task " << task.owner << "::" << task.name);
          task.execute();
        }
        catch (std::exception const & ex)
        {
          MLOG(WARN, "task " << task.owner << "::" << task.name << " failed: " << ex.what());
        }
      }
    }

    std::string const & kernel_t::get_name () const
    {
      return m_name;
    }

    void kernel_t::set_name (std::string const &n)
    {
      m_name = n;
    }
  }
}
