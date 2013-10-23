#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <csignal>

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>

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
      std::string name;
      fhg::plugin::task_t task;
      size_t ticks;
      int state;

      bool operator<(task_info_t const &rhs) const
      {
        return ticks < rhs.ticks;
      }

      task_info_t ( std::string const & o
                  , std::string const & n
                  , fhg::plugin::task_t t
                  , size_t num_ticks
                  )
        : owner(o)
        , name(n)
        , task(t)
        , ticks(num_ticks)
        , state (PENDING)
      {}

      task_info_t (const task_info_t &other)
        : owner(other.owner)
        , name(other.name)
        , task(other.task)
        , ticks(other.ticks)
        , state (other.state)
      {}

      task_info_t & operator= (const task_info_t &other)
      {
        owner = other.owner;
        name = other.name;
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
        if (state == PENDING)
        {
          try
          {
            task();
          }
          catch (...)
          {
            lock_type lck(m_mutex);
            state = CANCELLED;
            termination_condition.notify_all();
            throw;
          }
          state = DONE;
        }

        lock_type lck(m_mutex);
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
      typedef std::vector<std::string> search_path_t;
      typedef std::list<std::string> plugin_names_t;

      explicit
      kernel_t (std::string const &state_path = "");
      ~kernel_t ();

      int run ();
      int run_and_unload (bool do_unload);

      void stop ();
      void reset ();
      void wait_until_stopped ();

      void add_search_path (std::string const &);
      void clear_search_path ();
      void get_search_path (search_path_t &);

      int load_plugin (std::string const & entity);
      int load_plugin_by_name (std::string const & name);
      int load_plugin_from_file (std::string const & file);
      int unload_plugin (std::string const &name);
      bool is_plugin_loaded (std::string const &name);

      int handle_signal (int signum, siginfo_t *info, void *ctxt);

      fhg::plugin::Storage * storage();
      fhg::plugin::Storage * plugin_storage();

      void unload_all ();

      void schedule( std::string const &owner
                   , std::string const &name
                   , fhg::plugin::task_t
                   );
      void schedule( std::string const &owner
                   , std::string const &name
                   , fhg::plugin::task_t, size_t ticks
                   );
      plugin_t::ptr_t lookup_plugin(std::string const & name);

      template <typename Iface>
      Iface* lookup_plugin_as (std::string const & name)
      {
        plugin_t::ptr_t plugin (this->lookup_plugin (name));
        return
          dynamic_cast<Iface*>(plugin->get_plugin ());
      }

      time_t tick_time () const;

      std::string get(std::string const & key, std::string const &dflt) const;
      std::string put(std::string const & key, std::string const &value);

      void plugin_start_completed(std::string const & name, int);
      void plugin_failed(std::string const &name, int);

      void set_name (std::string const &n);
      std::string const & get_name () const;
    private:
      void initialize_storage ();
      void require_dependencies (fhg::core::plugin_t::ptr_t const &);
      void remove_pending_tasks (std::string const & owner);
      void notify_plugin_load (std::string const & name);

      void task_handler ();

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      typedef boost::shared_ptr<PluginKernelMediator> mediator_ptr;
      typedef std::map<std::string, mediator_ptr> plugin_map_t;
      typedef fhg::thread::queue<task_info_t, std::list> task_queue_t;
      typedef std::list<task_info_t> task_list_t;
      typedef std::map<std::string, std::string> config_t;

      int unload_plugin (plugin_map_t::iterator it);

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;
      mutable mutex_type m_mtx_incomplete_plugins;
      mutable mutex_type m_mtx_task_queue;
      mutable mutex_type m_mtx_pending_tasks;
      mutable condition_type m_task_eligible;
      mutable mutex_type m_mtx_config;

      std::string m_state_path;
      time_t m_tick_time;
      bool m_stop_requested;
      bool m_running;
      plugin_map_t   m_plugins;
      plugin_map_t   m_incomplete_plugins;
      plugin_names_t m_load_order;

      task_list_t m_pending_tasks;
      task_queue_t m_task_queue;
      config_t m_config;

      mutable mutex_type m_mtx_storage;
      fhg::plugin::Storage *m_storage;

      boost::thread m_task_handler;
      std::string m_name;
      search_path_t m_search_path;
      search_path_t m_failed_path_cache;

      fhg::util::thread::event<int> m_stopped;
    };
  }
}

#endif
