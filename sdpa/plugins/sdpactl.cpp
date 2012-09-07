#include <sys/types.h> // fork
#include <unistd.h> // fork
#include <sys/wait.h> // waitpid
#include <stdlib.h> // system

#include "sdpactl.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

typedef int (*ChildFunction)(const char*);

namespace helper
{
  static void close_fds ()
  {
    for (int fd = 0 ; fd < 1024 ; ++fd)
    {
      close (fd);
    }
  }

  static int do_run (const char *cmd)
  {
    int ec = 0;

    ec = system (cmd);

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

  static int do_start (const char *comp)
  {
    int ec = 0;

    if (0 == comp)
    {
      ec += do_run ("sdpa start gpi")  ? 1 : 0;
      ec += do_run ("sdpa start orch") ? 2 : 0;
      ec += do_run ("sdpa start agg")  ? 4 : 0;
      ec += do_run ("sdpa start drts") ? 8 : 0;
    }
    else
    {
      std::string cmd ("sdpa start ");
      cmd += comp;

      ec += do_run (cmd.c_str ());
    }
    return ec;
  }

  static int do_stop (const char *comp)
  {
    int ec = 0;
    if (0 == comp)
    {
      ec += do_run ("sdpa stop drts") ?  1 : 0;
      ec += do_run ("sdpa stop agg")  ?  2 : 0;
      ec += do_run ("sdpa stop orch") ?  4 : 0;
      ec += do_run ("sdpa stop gpi")  ?  8 : 0;
      ec += do_run ("sdpa stop kvs")  ? 16 : 0;
    }
    else
    {
      std::string cmd ("sdpa stop ");
      cmd += comp;

      ec += do_run (cmd.c_str ());
    }
    return ec;
  }

  static int do_status (const char *comp)
  {
    int ec = 0;

    std::string cmd ("sdpa status");
    if (comp)
    {
      cmd += " ";
      cmd += (const char *)(comp);
    }

    ec += do_run (cmd.c_str ());

    return ec;
  }

  static int run_in_child (ChildFunction fun, const char* arg)
  {
    pid_t child = fork ();
    if (child == 0)
    {
      close_fds ();
      int ec = 0;

      ec += fun(arg);

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
      pid_t ec = waitpid(child, &status, 0);

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
    int rc = helper::run_in_child (&helper::do_start, comp);
    sleep (m_time_to_sleep_after_startup);
    return rc;
  }

  int stop ()
  {
    return stop (0);
  }

  int stop (const char *comp)
  {
    return helper::run_in_child (&helper::do_stop, comp);
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
    return helper::run_in_child (&helper::do_status, comp);
  }
private:
  int m_time_to_sleep_after_startup;
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
