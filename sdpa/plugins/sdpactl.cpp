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

  static int do_start (void* time_to_sleep)
  {
    int ec = 0;
    ec += system("sdpa start gpi");
    ec += system("sdpa start orch");
    ec += system("sdpa start agg");
    ec += system("sdpa start nre");
    ec += system("sdpa start drts");
    if (time_to_sleep)
    {
      sleep(*(int*)(time_to_sleep));
    }
    return ec;
  }

  static int do_stop (void*)
  {
    int ec = 0;
    ec += system("sdpa stop drts");
    ec += system("sdpa stop nre");
    ec += system("sdpa stop agg");
    ec += system("sdpa stop orch");
    ec += system("sdpa stop gpi");
    ec += system("sdpa stop kvs");
    return ec;
  }

  static int do_restart (void* p)
  {
    int ec = 0;
    ec += do_stop (p);
    ec += do_start (p);
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
      return ec;
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
        return EFAULT;
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
