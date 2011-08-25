#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <cstring>

#include <fhglog/minimal.hpp>

#include <fhg/plugin/config.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <fhg/util/split.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

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
    kernel_t::kernel_t ()
      : m_tick_time (5 * 100 * 1000)
      , m_stop_requested (false)
    {}

    kernel_t::~kernel_t ()
    {
      // unload all non-static plugins according to dependency graph
      unload_all ();

      // warning:  we must  not have  any  tasks stored,  since we  would get  a
      // segfault due to dynamic symbols
      assert (m_task_queue.empty());
      assert (m_pending_tasks.empty());
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

      bool load_force (boost::lexical_cast<bool>(get("kernel.load.force", "0")));
      bool load_lazy (boost::lexical_cast<bool>(get("kernel.load.lazy", "1")));

      plugin_t::ptr_t p (plugin_t::create( file
                                         , load_force
                                         , (load_lazy?RTLD_LAZY:RTLD_NOW)
                                         )
                        );
      {
        lock_type plugins_lock (m_mtx_plugins);
        if (m_plugins.find(p->name()) != m_plugins.end())
        {
          // TODO: print descriptor of other plugin
          throw std::runtime_error
            ("another plugin with the same name is already loaded: " + p->name());
        }
      }

      check_dependencies(p);
      // create mediator
      mediator_ptr m(new PluginKernelMediator(p, this));

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
        }

        MLOG(TRACE, p->name() << " plugin loaded");

        for ( plugin_map_t::iterator it (m_plugins.begin())
            ; it != m_plugins.end()
            ; ++it
            )
        {
          mediator_ptr m = it->second;
          if (it->first != p->name())
            m->plugin()->handle_plugin_loaded(p->name());
        }
      }
      else if (rc == START_INCOMPLETE) // incomplete
      {
        lock_type plugins_lock (m_mtx_incomplete_plugins);
        m_incomplete_plugins.insert (std::make_pair(p->name(), m));

        MLOG(TRACE, "start of " << p->name() << " incomplete");
      }
      else
      {
        throw std::runtime_error
          ("plugin " + p->name() + " failed to start: " + std::string(strerror(-rc)));
      }

      return rc;
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

      LOG(TRACE, p->first << " plugin unloaded");

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

      m_plugins.erase (p);

      return rc;
    }

    void kernel_t::remove_pending_tasks(std::string const &owner)
    {
      lock_type lock_pending (m_mtx_pending_tasks);
      lock_type lock_task_q (m_mtx_task_queue);

      for ( task_queue_t::iterator it = m_task_queue.begin()
          ; it != m_task_queue.end()
          ; // done explicitly
          )
      {
        if (it->owner == owner)
        {
          it = m_task_queue.erase(it);
        }
        else
        {
          ++it;
        }
      }

      for ( task_queue_t::iterator it = m_pending_tasks.begin()
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

        for ( plugin_map_t::iterator it (m_plugins.begin())
            ; it != m_plugins.end()
            ; ++it
            )
        {
          mediator_ptr other = it->second;
          if (it->first != name)
            other->plugin()->handle_plugin_loaded(name);
        }
      }
    }

    void kernel_t::check_dependencies (plugin_t::ptr_t const &plugin)
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
          throw std::runtime_error("dependency not available: " + dep);
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
          for ( plugin_map_t::iterator it (m_plugins.begin())
              ; it != m_plugins.end()
              ; ++it
              )
          {
            if (unload_plugin (it) < 0) continue;
          }
        }
      }
    }

    void kernel_t::schedule( std::string const &owner
                           , fhg::plugin::task_t task
                           )
    {
      schedule (owner, task, 0);
    }

    void kernel_t::schedule( std::string const &owner
                           , fhg::plugin::task_t task
                           , size_t ticks
                           )
    {
      lock_type lock(m_mtx_pending_tasks);
      m_pending_tasks.push_back(task_info_t(owner, task, ticks));
    }

    void kernel_t::stop ()
    {
      m_stop_requested = true;
    }

    void kernel_t::reset ()
    {
      m_stop_requested = false;
    }

    int kernel_t::run ()
    {
      struct timeval tv_start;
      struct timeval tv_diff;
      struct timeval tv_end;

      while (!m_stop_requested)
      {
        gettimeofday(&tv_start, 0);

        { /* decrement tick count of pending tasks and move them */
          lock_type lock_pending (m_mtx_pending_tasks);
          lock_type lock_task_q (m_mtx_task_queue);
          for ( task_queue_t::iterator it = m_pending_tasks.begin()
              ; it != m_pending_tasks.end()
              ;
              )
          {
            if (0 == it->ticks)
            {
              m_task_queue.push_back (*it);
              it = m_pending_tasks.erase (it);
            }
            else
            {
              --it->ticks;
              ++it;
            }
          }
        }

        { // run due tasks
          lock_type lock_task_q (m_mtx_task_queue);
          while (! m_task_queue.empty())
          {
            task_info_t task = m_task_queue.front();
            m_task_queue.pop_front();

            try
            {
              LOG(TRACE, "executing task of plugin " << task.owner);
              task.execute();
            }
            catch (std::exception const & ex)
            {
              MLOG(WARN, "task of plugin " << task.owner << " failed: " << ex.what());
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
  }
}
