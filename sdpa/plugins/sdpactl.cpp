#include <sys/types.h> // fok
#include <unistd.h> // fork
#include <sys/wait.h> // waitpid
#include <stdlib.h> // system

#include "sdpactl.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

typedef int (*ChildFunction)(void*);

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

  static int do_start (void* time_to_sleep)
  {
    int ec = 0;
    ec += do_run ("sdpa start gpi")  ? 1 : 0;
    ec += do_run ("sdpa start orch") ? 2 : 0;
    ec += do_run ("sdpa start agg")  ? 4 : 0;
    ec += do_run ("sdpa start drts") ? 8 : 0;
    if (time_to_sleep)
    {
      sleep(*(int*)(time_to_sleep));
    }
    return ec;
  }

  static int do_stop (void*)
  {
    int ec = 0;
    ec += do_run ("sdpa stop drts") ?  1 : 0;
    ec += do_run ("sdpa stop agg")  ?  2 : 0;
    ec += do_run ("sdpa stop orch") ?  4 : 0;
    ec += do_run ("sdpa stop gpi")  ?  8 : 0;
    ec += do_run ("sdpa stop kvs")  ? 16 : 0;
    return ec;
  }

  static int do_restart (void* p)
  {
    int ec = 0;
    ec += do_stop (p);
    ec += do_start (p);
    return ec;
  }

  static int do_status (void *p)
  {
    int ec = 0;

    std::string cmd ("sdpa status");
    if (p)
    {
      cmd += " ";
      cmd += (const char *)(p);
    }

    ec += do_run (cmd.c_str ());

    return ec;
  }

  static int run_in_child (ChildFunction fun, void* arg)
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
    return helper::run_in_child ( &helper::do_start
                                , &m_time_to_sleep_after_startup
                                );
  }

  int restart ()
  {
    return helper::run_in_child ( &helper::do_restart
                                , &m_time_to_sleep_after_startup
                                );
  }

  int stop ()
  {
    return helper::run_in_child ( &helper::do_stop
                                , 0
                                );
  }

  int status ()
  {
    return helper::run_in_child ( &helper::do_status
                                , 0
                                );
  }

  int status (const char * comp)
  {
    return helper::run_in_child ( &helper::do_status
                                , (void*)comp
                                );
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
