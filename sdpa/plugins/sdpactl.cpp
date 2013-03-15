#include <sys/types.h> // fork
#include <unistd.h> // fork
#include <sys/wait.h> // waitpid
#include <stdlib.h> // system

#include "sdpactl.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

struct child_arg_t
{
  const char *comp;
  const char *state;
  const char *config;
};

typedef int (*ChildFunction)(child_arg_t *);

namespace helper
{
  static void close_fds ()
  {
    for (int fd = 0 ; fd < 1024 ; ++fd)
    {
      close (fd);
    }
  }

  static int do_run (std::string const &cmd)
  {
    int ec = 0;

    ec = system (cmd.c_str ());

    if (WIFEXITED(ec))
    {
      return WEXITSTATUS(ec);
    }
    else if (WIFSIGNALED(ec))
    {
      return 42;
    }
    else
    {
      return 127;
    }
  }

  static int do_start (child_arg_t *arg)
  {
    const char *comp = arg->comp;
    int ec = 0;

    std::string cmd;

    cmd = "sdpa";
    if (arg->state)
    {
      cmd += " -s "; cmd += arg->state;
    }
    if (arg->config)
    {
      cmd += " -c "; cmd += arg->config;
    }
    cmd += " start";

    if (0 == comp)
    {
      std::string tmp;

      tmp = cmd + " gpi";
      ec += do_run (tmp)  ? 1 : 0;

      tmp = cmd + " orch";
      ec += do_run (tmp)  ? 2 : 0;

      tmp = cmd + " agg";
      ec += do_run (tmp)  ? 4 : 0;

      tmp = cmd + " drts";
      ec += do_run (tmp)  ? 8 : 0;
    }
    else
    {
      std::string tmp;

      tmp  = cmd + " ";
      tmp += comp;

      ec += do_run (tmp);
    }
    return ec;
  }

  static int do_stop (child_arg_t *arg)
  {
    const char *comp = arg->comp;
    int ec = 0;

    std::string cmd;

    cmd = "sdpa";
    if (arg->state)
    {
      cmd += " -s "; cmd += arg->state;
    }
    if (arg->config)
    {
      cmd += " -c "; cmd += arg->config;
    }
    cmd += " stop";

    if (0 == comp)
    {
      std::string tmp;

      tmp = cmd + " drts";
      ec += do_run (tmp)  ? 8 : 0;

      tmp = cmd + " agg";
      ec += do_run (tmp)  ? 4 : 0;

      tmp = cmd + " orch";
      ec += do_run (tmp)  ? 2 : 0;

      tmp = cmd + " gpi";
      ec += do_run (tmp)  ? 1 : 0;

      tmp = "sleep 5";
      do_run (tmp);

      tmp = cmd + " kvs";
      ec += do_run (tmp)  ?16 : 0;
    }
    else
    {
      std::string tmp;

      tmp  = cmd + " ";
      tmp += comp;

      ec += do_run (tmp);
    }
    return ec;
  }

  static int do_status (child_arg_t *arg)
  {
    const char *comp = arg->comp;
    int ec = 0;

    std::string cmd;

    cmd = "sdpa";
    if (arg->state)
    {
      cmd += " -s "; cmd += arg->state;
    }
    if (arg->config)
    {
      cmd += " -c "; cmd += arg->config;
    }
    cmd += " status";

    if (0 == comp)
    {
      std::string tmp;

      tmp = cmd + " gpi";
      ec += do_run (tmp)  ? 1 : 0;

      tmp = cmd + " orch";
      ec += do_run (tmp)  ? 2 : 0;

      tmp = cmd + " agg";
      ec += do_run (tmp)  ? 4 : 0;

      tmp = cmd + " drts";
      ec += do_run (tmp)  ? 8 : 0;

      tmp = cmd + " kvs";
      ec += do_run (tmp)  ?16 : 0;
    }
    else
    {
      std::string tmp;

      tmp  = cmd + " ";
      tmp += comp;

      ec += do_run (tmp);
    }
    return ec;
  }

  static int run_in_child (ChildFunction fun, child_arg_t *arg)
  {
    pid_t child = fork ();
    if (child == 0)
    {
      close_fds ();
      int ec = 0;

      ec += fun (arg);

      _exit (ec);

      // never reached
      //   return ec;
    }
    else if (child < 0)
    {
      int ec = errno;
      MLOG(ERROR, "could not fork: " << strerror(ec));
      return -ec;
    }
    else
    {
      int status;
      waitpid(child, &status, 0);

      if (WIFEXITED(status))
      {
        return WEXITSTATUS(status);
      }
      else if (WIFSIGNALED(status))
      {
        return WTERMSIG(status);
      }
      else
      {
        return 127;
      }
    }
  }
}

class ControlImpl : FHG_PLUGIN
                  , public sdpa::Control
{
public:
  FHG_PLUGIN_START()
  {
    m_time_to_sleep_after_startup =
      fhg_kernel()->get<int>("time_to_sleep", "5");

    m_state = fhg_kernel ()->get ("state", "");
    if (m_state.size ())
    {
      MLOG (INFO, "using state directory: " << m_state);
    }

    m_config = fhg_kernel ()->get ("config", "");
    if (m_config.size ())
    {
      MLOG (INFO, "using config file: " << m_config);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int start ()
  {
    return start (0);
  }

  int start (const char *comp)
  {
    struct child_arg_t arg =
      { comp
      , m_state.size () ? m_state.c_str () : 0
      , m_config.size () ? m_config.c_str () : 0
      };

    int rc = helper::run_in_child (&helper::do_start, &arg);
    sleep (m_time_to_sleep_after_startup);
    return rc;
  }

  int stop ()
  {
    return stop (0);
  }

  int stop (const char *comp)
  {
    struct child_arg_t arg =
      { comp
      , m_state.size () ? m_state.c_str () : 0
      , m_config.size () ? m_config.c_str () : 0
      };

    return helper::run_in_child (&helper::do_stop, &arg);
  }

  int restart ()
  {
    return restart (0);
  }

  int restart (const char *comp)
  {
    int rc = 0;

    rc += start (comp);
    sleep (m_time_to_sleep_after_startup);
    rc += stop (comp);

    return rc;
  }

  int status ()
  {
    return status (0);
  }

  int status (const char * comp)
  {
    struct child_arg_t arg =
      { comp
      , m_state.size () ? m_state.c_str () : 0
      , m_config.size () ? m_config.c_str () : 0
      };

    return helper::run_in_child (&helper::do_status, &arg);
  }
private:
  int m_time_to_sleep_after_startup;
  std::string m_state;
  std::string m_config;
};

EXPORT_FHG_PLUGIN( sdpactl
                 , ControlImpl
                 , "sdpactl"
                 , "provides control functions for SDPA"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
