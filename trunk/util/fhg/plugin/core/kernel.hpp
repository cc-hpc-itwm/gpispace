#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/core/plugin_kernel_mediator.hpp>

namespace fhg
{
  namespace core
  {
    struct task_info_t
    {
      enum
        {
          PENDING = 0
        , DONE = 1
        , CANCELLED = 2
        };

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      std::string owner;
      fhg::plugin::task_t task;
      size_t ticks;
      int state;

      bool operator<(task_info_t const &rhs) const
      {
        return ticks < rhs.ticks;
      }

      task_info_t (std::string const & o, fhg::plugin::task_t t, size_t num_ticks)
        : owner(o)
        , task(t)
        , ticks(num_ticks)
        , state (PENDING)
      {}

      task_info_t (const task_info_t &other)
        : owner(other.owner)
        , task(other.task)
        , ticks(other.ticks)
        , state (other.state)
      {}

      task_info_t & operator= (const task_info_t &other)
      {
        owner = other.owner;
        task = other.task;
        ticks = other.ticks;
        state = other.state;
        return *this;
      }

      void cancel ()
      {
        lock_type lck(m_mutex);
        if (state == PENDING)
        {
          state = CANCELLED;
        }
        termination_condition.notify_all();
      }

      void execute ()
      {
        lock_type lck(m_mutex);
        if (state == PENDING)
        {
          try
          {
            task();
          }
          catch (...)
          {
            state = CANCELLED;
            termination_condition.notify_all();
            throw;
          }
          state = DONE;
        }
        termination_condition.notify_all();
      }

      void wait ()
      {
        lock_type lck(m_mutex);
        while (state == PENDING) termination_condition.wait(lck);
      }
    private:
      mutable mutex_type m_mutex;
      mutable condition_type termination_condition;
    };

    class kernel_t
    {
    public:
      kernel_t ();
      ~kernel_t ();

      int run ();
      void stop ();
      void reset ();

      int load_plugin (std::string const & file);
      int unload_plugin (std::string const &name);

      void unload_all ();

      void schedule(std::string const &owner, fhg::plugin::task_t);
      void schedule(std::string const &owner, fhg::plugin::task_t, size_t ticks);
      plugin_t::ptr_t lookup_plugin(std::string const & name);

      time_t tick_time () const;

      std::string get(std::string const & key, std::string const &dflt) const;
      std::string put(std::string const & key, std::string const &value);
    private:
      void check_dependencies (fhg::core::plugin_t::ptr_t const &);

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      typedef boost::shared_ptr<PluginKernelMediator> mediator_ptr;
      typedef std::map<std::string, mediator_ptr> plugin_map_t;
      typedef std::list<task_info_t> task_queue_t;
      typedef std::map<std::string, std::string> config_t;

      int unload_plugin (plugin_map_t::iterator it);

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;
      mutable mutex_type m_mtx_incomplete_plugins;
      mutable mutex_type m_mtx_task_queue;
      mutable mutex_type m_mtx_pending_tasks;
      mutable condition_type m_task_eligible;
      mutable mutex_type m_mtx_config;

      time_t m_tick_time;
      bool m_stop_requested;
      plugin_map_t m_plugins;
      plugin_map_t m_incomplete_plugins;

      task_queue_t m_task_queue;
      task_queue_t m_pending_tasks;
      config_t m_config;
    };
  }
}

#endif
