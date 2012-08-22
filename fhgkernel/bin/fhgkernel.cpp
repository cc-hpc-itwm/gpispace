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

 LOG(ERROR, log_message.str());

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
  LOG(INFO, "trying to continue anyways");
}

// END OF BORROWED CODE

static const int EX_STILL_RUNNING = 4;
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
  std::string pidfile;
  std::string kernel_name ("fhgkernel");
  bool daemonize (false);
  fhg::core::kernel_t::search_path_t search_path;

  desc.add_options()
    ("help,h", "this message")
    ("verbose,v", "be verbose")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
    ("state,S", po::value<std::string>(&state_path), "state directory to use")
    ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
    ("daemonize", "daemonize after all checks were successful")
    ( "keep-going,k", "just log errors, but do not refuse to start")
    ( "load,l"
    , po::value<std::vector<std::string> >(&mods_to_load)
    , "modules to load"
    )
    ( "add-search-path,L", po::value<fhg::core::kernel_t::search_path_t>(&search_path)
    , "add a path to the search path for plugins"
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

  if (vm.count("daemonize"))
    daemonize = true;

  keep_going = vm.count("keep-going") != 0;

  int pidfile_fd = -1;

  if (not pidfile.empty())
  {
    pidfile_fd = open(pidfile.c_str(), O_CREAT|O_RDWR, 0640);
    if (pidfile_fd < 0)
    {
      LOG( ERROR, "could not open pidfile for writing: "
         << strerror(errno)
         );
      exit(EXIT_FAILURE);
    }
  }

  // everything is fine so far, daemonize
  if (daemonize)
  {
    if (pid_t child = fork())
    {
      if (child == -1)
      {
        LOG(ERROR, "could not fork: " << strerror(errno));
        exit(EXIT_FAILURE);
      }
      else
      {
        exit (EXIT_SUCCESS);
      }
    }
    setsid();
    close(0); close(1); close(2);
    int fd = open("/dev/null", O_RDWR);
    if (-1 == dup(fd))
    {
      LOG(WARN, "could not duplicate /dev/null to stdout: " << strerror(errno));
    }
    if (-1 == dup(fd))
    {
      LOG(WARN, "could not duplicate /dev/null to stdout: " << strerror(errno));
    }
  }

  if (pidfile_fd >= 0)
  {
    if (lockf(pidfile_fd, F_TLOCK, 0) < 0)
    {
      LOG( ERROR, "could not lock pidfile: "
         << strerror(errno)
         );
      exit(EX_STILL_RUNNING);
    }

    char buf[32];
    if (0 != ftruncate(pidfile_fd, 0))
    {
      LOG(WARN, "could not truncate pidfile: " << strerror(errno));
    }
    snprintf(buf, sizeof(buf), "%d\n", getpid());
    if (write(pidfile_fd, buf, strlen(buf)) <= 0)
    {
      LOG(ERROR, "could not write pid: " << strerror(errno));
      exit(EXIT_FAILURE);
    }
    fsync(pidfile_fd);
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
      int ec = kernel->load_plugin (p);
      if (ec != 0)
        throw std::runtime_error (strerror (ec));
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
