// alexander.petry@itwm.fraunhofer.de

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/core/license.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/get_home_dir.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/program_info.h>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/thread/pool.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

namespace
{
  void crit_err_hdlr ( int sig_num, siginfo_t* info, void* context
                     , fhg::log::Logger::ptr_t logger
                     )
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

   LLOG (ERROR, logger, fhg::util::make_backtrace (log_message.str()));

   _exit (EXIT_FAILURE);
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

  fhg::core::kernel_t kernel (state_path, kernel_name, search_path);

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
      kernel.put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      int ec = kernel.load_plugin (p);
      if (ec != 0)
        throw std::runtime_error (strerror (ec));
    }
    catch (std::exception const &ex)
    {
      LLOG (ERROR, logger, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        throw;
      }
    }
  }

  fhg::util::signal_handler_manager signal_handlers;
  signal_handlers.add (SIGSEGV, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
  signal_handlers.add (SIGBUS, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
  signal_handlers.add (SIGABRT, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
  signal_handlers.add (SIGFPE, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));

  signal_handlers.add (SIGTERM, boost::bind (&fhg::core::kernel_t::stop, &kernel));
  signal_handlers.add (SIGINT, boost::bind (&fhg::core::kernel_t::stop, &kernel));

  return kernel.run_and_unload (false);
}
