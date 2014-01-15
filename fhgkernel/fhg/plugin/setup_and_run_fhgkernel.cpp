// alexander.petry@itwm.fraunhofer.de

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include <ucontext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include <vector>
#include <string>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/get_home_dir.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/program_info.h>
#include <fhg/util/thread/pool.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <fhg/plugin/core/license.hpp>

#include <fhglog/LogMacros.hpp>

namespace
{
  fhg::log::Logger::ptr_t GLOBAL_logger;

  void crit_err_hdlr(int sig_num, siginfo_t* info, void* context)
  {
   sigcontext* mcontext (static_cast<sigcontext*> (static_cast<void*>
                          (&(static_cast<ucontext*> (context)->uc_mcontext))
                        ));

#if __WORDSIZE == 32
   unsigned long caller_address (mcontext->eip);
#else
#if __WORDSIZE == 64
   unsigned long caller_address (mcontext->rip);
#else
#error Unable to get caller_address on this architecture.
#endif
#endif

   fprintf ( stderr
           , "signal %d (%s), address is %p from %p\n"
           , sig_num, strsignal(sig_num), info->si_addr, (void*)caller_address
           );

   std::ostringstream log_message;
   log_message << "received signal "
               << sig_num << " (" << strsignal(sig_num) << "),"
               << " address is " << (void*)info->si_addr
               << " from " << (void*)caller_address;

   LLOG (ERROR, GLOBAL_logger, fhg::util::make_backtrace (log_message.str()));

   _exit(EXIT_FAILURE);
  }

  fhg::core::kernel_t *kernel = 0;

  void shutdown_kernel ()
  {
    if (kernel)
    {
      kernel->stop();
    }
  }

  void handle_sig_pipe() {}

  void sigterm_hdlr(int sig_num, siginfo_t * info, void * ucontext)
  {
    if (kernel)
    {
      if (0 == kernel->handle_signal (sig_num, info, ucontext))
      {
        kernel->stop();
      }
    }
  }

  void sigpipe_hdlr(int sig_num, siginfo_t * info, void * ucontext)
  {
    if (kernel)
    {
      kernel->schedule("kernel", "sigpipe", &handle_sig_pipe);
      kernel->handle_signal (sig_num, info, ucontext);
    }
  }

  void sigint_hdlr(int sig_num, siginfo_t *info, void *ucontext)
  {
    if (kernel)
    {
      if (0 == kernel->handle_signal (sig_num, info, ucontext))
      {
        kernel->stop();
      }
    }
  }

  void sigusr_hdlr(int sig_num, siginfo_t * info, void * ucontext)
  {
    if (kernel)
    {
      kernel->handle_signal (sig_num, info, ucontext);
    }
  }

  void install_signal_handler
    (int signum, void (*handler) (int, siginfo_t*, void*), bool restartable)
  {
    struct sigaction sigact;
    sigact.sa_sigaction = handler;
    sigact.sa_flags = restartable ? (SA_RESTART | SA_SIGINFO) : SA_SIGINFO;

    if (sigaction (signum, &sigact, NULL) < 0)
    {
      throw std::runtime_error ("unable to set up signal handler for " + std::string (strsignal (signum)));
    }
  }
}

int setup_and_run_fhgkernel ( bool daemonize
                            , bool keep_going
                            , std::vector<std::string> mods_to_load
                            , std::vector<std::string> config_vars
                            , std::string state_path
                            , std::string pidfile
                            , std::string kernel_name
                            , fhg::core::kernel_t::search_path_t search_path
                            , fhg::log::Logger::ptr_t logger
                            )
{
  assert (!kernel);

  fhg::plugin::magically_check_license (logger);

  if (not pidfile.empty())
  {
    fhg::util::pidfile_writer const pidfile_writer (pidfile);

    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    pidfile_writer.write();
  }
  else
  {
    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }
  }

  install_signal_handler (SIGSEGV, &crit_err_hdlr, true);
  install_signal_handler (SIGBUS, &crit_err_hdlr, true);
  install_signal_handler (SIGABRT, &crit_err_hdlr, true);
  install_signal_handler (SIGFPE, &crit_err_hdlr, true);

  install_signal_handler (SIGTERM, &sigterm_hdlr, true);

  install_signal_handler (SIGINT, &sigint_hdlr, true);

  install_signal_handler (SIGPIPE, &sigpipe_hdlr, false);

  install_signal_handler (SIGUSR1, &sigusr_hdlr, true);
  install_signal_handler (SIGUSR2, &sigusr_hdlr, true);

  kernel = new fhg::core::kernel_t (state_path);
  kernel->set_name (kernel_name);
  BOOST_FOREACH (std::string const & p, search_path)
  {
    kernel->add_search_path (p);
  }

  BOOST_FOREACH (std::string const & p, config_vars)
  {
    typedef std::pair<std::string,std::string> key_val_t;
    key_val_t kv (fhg::util::split_string(p, "="));
    if (kv.first.empty())
    {
      LLOG (WARN, logger, "invalid config variable: must not be empty");
    }
    else
    {
      DLLOG (TRACE, logger, "setting " << kv.first << " to " << kv.second);
      kernel->put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      int ec = kernel->load_plugin (p);
      if (ec != 0)
        throw std::runtime_error (strerror (ec));
    }
    catch (std::exception const &ex)
    {
      LLOG (ERROR, logger, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        kernel->stop();
        kernel->unload_all();
        return EXIT_FAILURE;
      }
    }
  }

  atexit(&shutdown_kernel);

  int rc = kernel->run_and_unload (false);

  DLLOG (TRACE, logger, "shutting down... (" << rc << ")");

  kernel->unload_all();

  fhg::core::kernel_t *tmp_kernel = kernel;
  kernel = 0;
  delete tmp_kernel;

  return rc;
}
