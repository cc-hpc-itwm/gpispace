#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <vector>
#include <string>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/util/split.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

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

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
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

 LOG(ERROR, log_message.str());

 free(messages);

 _exit(EXIT_FAILURE);
}

// END OF BORROWED CODE

static fhg::core::kernel_t *kernel = 0;

static void shutdown_kernel ()
{
  if (kernel) kernel->stop();
}

static void handle_sig_pipe() {}
static void handle_sig_child()
{
  waitpid(-1, 0, WNOHANG);
}

void sigterm_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  shutdown_kernel();
}

void sigpipe_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  if (kernel)
    kernel->schedule("kernel", "sigpipe", &handle_sig_pipe);
}

void sigchild_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
  if (kernel)
    kernel->schedule("kernel", "wait_on_child", &handle_sig_child);
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

  if (sigaction(SIGABRT, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGABRT, strsignal(SIGABRT));

    exit(EXIT_FAILURE);
  }

  // install non-critical handlers
  sigact.sa_sigaction = sigterm_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  if (sigaction(SIGTERM, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGTERM, strsignal(SIGTERM));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigchild_hdlr;
  if (sigaction(SIGCHLD, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGCHLD, strsignal(SIGCHLD));

    exit(EXIT_FAILURE);
  }

  sigact.sa_sigaction = sigpipe_hdlr;
  if (sigaction(SIGPIPE, &sigact, (struct sigaction *)NULL) != 0)
  {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
           SIGPIPE, strsignal(SIGPIPE));

    exit(EXIT_FAILURE);
  }
}

int main(int ac, char **av)
{
  FHGLOG_SETUP(ac,av);

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> mods_to_load;
  std::vector<std::string> config_vars;
  bool keep_going (false);
  std::string state_path;

  desc.add_options()
    ("help,h", "this message")
    ("verbose,v", "be verbose")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
    ("state,S", po::value<std::string>(&state_path), "state directory to use")
    ( "keep-going,k", "just log errors, but do not refuse to start")
    ( "load,l"
    , po::value<std::vector<std::string> >(&mods_to_load)
    , "modules to load"
    )
    ;

  po::positional_options_description p;
  p.add("load", -1);

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(ac, av)
             . options(desc).positional(p).run()
             , vm
             );
  }
  catch (std::exception const &ex)
  {
    std::cerr << "invalid command line: " << ex.what() << std::endl;
    std::cerr << "use " << av[0] << " --help to get a list of options" << std::endl;
    return EXIT_FAILURE;
  }
  po::notify (vm);

  if (vm.count("help"))
  {
    std::cout << av[0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  kernel = new fhg::core::kernel_t (state_path);

  install_signal_handler();

  keep_going = vm.count("keep-going") != 0;

  BOOST_FOREACH (std::string const & p, config_vars)
  {
    typedef std::pair<std::string,std::string> key_val_t;
    key_val_t kv (fhg::util::split_string(p, "="));
    if (kv.first.empty())
    {
      MLOG(WARN, "invalid config variable: must not be empty");
    }
    else
    {
      MLOG(TRACE, "setting " << kv.first << " to " << kv.second);
      kernel->put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      kernel->load_plugin (p);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        kernel->stop();
        kernel->unload_all();
        return EXIT_FAILURE;
      }
    }
  }

  atexit(&shutdown_kernel);
  int rc = kernel->run();
  MLOG(TRACE, "shutting down... (" << rc << ")");
  MLOG(TRACE, "killing children...");

  kill (0, SIGTERM);

  kernel->unload_all();

  delete kernel;
  kernel = 0;

  return rc;
}
