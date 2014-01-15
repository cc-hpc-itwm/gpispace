// alexander.petry@itwm.fraunhofer.de

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

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
}

// the following code was borrowed and slightly adapted from:
//
//   http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
//
// by: jschmier, http://stackoverflow.com/users/203667/jschmier

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

void log_backtrace(int sig_num, siginfo_t * info, void * ucontext)
{
 void *             array[50];
 void *             caller_address;
 char **            messages;
 int                size, i;
 sig_ucontext_t *   uc;

 uc = (sig_ucontext_t *)ucontext;

#if __WORDSIZE == 32
 /* Get the address at the time the signal was raised from the EIP (x86) */
 caller_address = (void *) uc->uc_mcontext.eip;
#else /* __WORDSIZE == 64 */
 caller_address = (void *) uc->uc_mcontext.rip;
#endif /* __WORDSIZE == 64 */

 fprintf(stderr, "signal %d (%s), address is %p from %p\n",
  sig_num, strsignal(sig_num), info->si_addr,
  (void *)caller_address);

 size = backtrace(array, 50);

 /* overwrite sigaction with caller's address */
 array[1] = caller_address;

 messages = backtrace_symbols(array, size);

 /* skip first stack frame (points here) */
 for (i = 1; i < size && messages != NULL; ++i)
 {
  fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
 }

 std::ostringstream log_message;
 log_message << "received signal "
             << sig_num << " (" << strsignal(sig_num) << "),"
             << " address is " << (void*)info->si_addr
             << " from " << (void*)(caller_address)
             << std::endl
   ;
 for (i = 1; i < size && messages != NULL; ++i)
 {
   log_message << "[bt]: (" << i << ") " << messages[i] << std::endl;
 }

 LLOG (ERROR, GLOBAL_logger, log_message.str());

 free(messages);
}

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  log_backtrace (sig_num, info, ucontext);
  _exit(EXIT_FAILURE);
}

void noncrit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  log_backtrace (sig_num, info, ucontext);
  LLOG (INFO, GLOBAL_logger, "trying to continue anyways");
}

// END OF BORROWED CODE

static const int EX_STILL_RUNNING = 4;
static fhg::core::kernel_t *kernel = 0;

static void shutdown_kernel ()
{
  if (kernel) kernel->stop();
}

static void handle_sig_pipe() {}

void sigterm_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  if (kernel)
  {
    if (0 == kernel->handle_signal (sig_num, info, ucontext))
    {
      shutdown_kernel ();
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
      shutdown_kernel ();
    }
  }
}

static void sigusr_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  if (kernel)
  {
    kernel->handle_signal (sig_num, info, ucontext);
  }
}

void install_signal_handler()
{
  struct sigaction sigact;

  // install critical handlers
  sigact.sa_sigaction = crit_err_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGSEGV, strsignal(SIGSEGV));

    exit(EXIT_FAILURE);
  }

  if (sigaction(SIGBUS, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGBUS, strsignal(SIGBUS));

    exit(EXIT_FAILURE);
  }

  if (sigaction(SIGABRT, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGABRT, strsignal(SIGABRT));

    exit(EXIT_FAILURE);
  }

  if (sigaction(SIGFPE, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGFPE, strsignal(SIGFPE));

    exit(EXIT_FAILURE);
  }

  // install non-critical handlers
  //sigact.sa_sigaction = noncrit_err_hdlr;
  //sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  // install informative handlers
  sigact.sa_sigaction = sigterm_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  if (sigaction(SIGTERM, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGTERM, strsignal(SIGTERM));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigint_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  if (sigaction(SIGINT, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGINT, strsignal(SIGINT));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigpipe_hdlr;
  sigact.sa_flags = SA_SIGINFO;
  if (sigaction(SIGPIPE, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGPIPE, strsignal(SIGPIPE));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigusr_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction(SIGUSR1, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGUSR1, strsignal(SIGUSR1));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigusr_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction(SIGUSR2, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGUSR2, strsignal(SIGUSR2));

    exit(EXIT_FAILURE);
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

  install_signal_handler();

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
