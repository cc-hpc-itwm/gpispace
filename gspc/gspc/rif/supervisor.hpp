#ifndef GSPC_RIF_SUPERVISOR_HPP
#define GSPC_RIF_SUPERVISOR_HPP

#include <list>
#include <string>

#include <boost/signals2/signal.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <gspc/rif/types.hpp>
#include <gspc/rif/process_handler.hpp>

namespace gspc
{
  namespace rif
  {
    struct child_descriptor_t
    {
      enum restart_mode_e
        {
          RESTART_ALWAYS
        , RESTART_NEVER
        , RESTART_ONLY_IF_FAILED
        };
      enum shutdown_mode_e
        {
          SHUTDOWN_KILL
        , SHUTDOWN_WITH_TIMEOUT
        , SHUTDOWN_INFINITY
        };

      child_descriptor_t ()
      {}

      child_descriptor_t ( std::string const &name
                         , argv_t const &argv
                         , env_t const &env
                         , restart_mode_e restart_mode = RESTART_ALWAYS
                         , shutdown_mode_e shutdown_mode = SHUTDOWN_KILL
                         , int timeout = 0
                         )
        : name (name)
        , argv (argv)
        , env (env)
        , restart_mode (restart_mode)
        , shutdown_mode (shutdown_mode)
        , timeout (timeout)
        , max_restarts (0)
        , max_start_time (0)
      {}

      std::string     name;
      argv_t          argv;
      env_t           env;
      restart_mode_e  restart_mode;
      shutdown_mode_e shutdown_mode;
      int             timeout;

      size_t          max_restarts;
      size_t          max_start_time;
    };

    class manager_t;

    class supervisor_t : public process_handler_t
    {
    public:
      typedef std::list<child_descriptor_t> child_descriptor_list_t;

      struct error_info_t
      {
        int         status;
        time_t      tstamp;
        std::string stdout;
        std::string stderr;
      };

      struct child_info_t
      {
        child_descriptor_t descriptor;
        proc_t             proc;
        time_t             started;
        size_t             trial;
        bool               restart;
        std::list<error_info_t> errors;
      };

      struct child_t
      {
        enum state_t
          {
            CHILD_TERMINATED
          , CHILD_RUNNING
          , CHILD_STARTING
          , CHILD_TERMINATING
          };

        child_info_t                info;

        state_t                     state;
        mutable boost::shared_mutex mutex;
      };

      boost::signals2::signal<void ()>                     onSupervisorStopped;
      boost::signals2::signal<void ()>                     onSupervisorStarted;
      boost::signals2::signal<void (child_info_t const &)> onChildFailed;
      boost::signals2::signal<void (child_info_t const &)> onChildStarted;
      boost::signals2::signal<void (child_info_t const &)> onChildTerminated;

      explicit
      supervisor_t ( manager_t & process_manager
                   , size_t max_restarts = 1
                   , size_t max_time = 30
                   , child_descriptor_list_t const & children
                   = child_descriptor_list_t ()
                   );

      ~supervisor_t ();

      void start ();
      void stop ();

      int add_child (child_descriptor_t const &child);
      int remove_child (std::string const &name);
      int restart_child (std::string const &name);
      int terminate_child (std::string const &name);

      std::list<std::string> get_children () const;

      child_info_t const &get_child_info (std::string const &name) const;
    private:
      typedef boost::shared_lock<boost::shared_mutex> shared_lock;
      typedef boost::unique_lock<boost::shared_mutex> unique_lock;

      typedef std::list<child_t*> child_list_t;

      void onStateChange (proc_t p, process_state_t s);
      void handle_terminated_child (child_t *child);
      int start_child (std::string const &name);

      const child_t *lookup_child (std::string const &name) const;
      child_t *lookup_child (std::string const &name);

      const child_t *lookup_child (const proc_t &pid) const;
      child_t *lookup_child (const proc_t &pid);

      mutable boost::shared_mutex m_mutex;

      manager_t   &m_process_manager;
      size_t       m_max_restarts;
      size_t       m_max_start_time;
      child_list_t m_children;
    };
  }
}

#endif
