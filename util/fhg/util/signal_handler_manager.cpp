// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/signal_handler_manager.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#include <stdio.h>

namespace fhg
{
  namespace util
  {
    namespace
    {
      boost::mutex GLOBAL_manager_mutex;
      signal_handler_manager* GLOBAL_manager;

      void signal_handler (int sig_num, siginfo_t* info, void* context)
      {
        boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
        if (GLOBAL_manager)
        {
          GLOBAL_manager->handle (sig_num, info, context);
        }
      }
    }

    signal_handler_manager::signal_handler_manager()
    {
      boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
      assert (!GLOBAL_manager);
      GLOBAL_manager = this;
    }

    signal_handler_manager::~signal_handler_manager()
    {
      boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
      assert (GLOBAL_manager);
      GLOBAL_manager = NULL;

      BOOST_FOREACH (int sig_num, _handlers | boost::adaptors::map_keys)
      {
        signal (sig_num, SIG_DFL);
      }
    }

    void signal_handler_manager::add
      (int sig_num, boost::function<void (int, siginfo_t*, void*)> fun)
    {
      boost::mutex::scoped_lock const _ (_handler_mutex);

      if (_handlers.find (sig_num) == _handlers.end())
      {
        struct sigaction sigact;
        sigact.sa_sigaction = signal_handler;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        if (sigaction (sig_num, &sigact, NULL) < 0)
        {
          throw std::runtime_error ("unable to set up signal handler for " + std::string (strsignal (sig_num)));
        }
      }

      _handlers[sig_num].push_back (fun);
    }

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

    void signal_handler_manager::add_log_backtrace_and_exit_for_critical_errors
      (fhg::log::Logger::ptr_t logger)
    {
      add (SIGSEGV, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
      add (SIGBUS, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
      add (SIGABRT, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
      add (SIGFPE, boost::bind (&crit_err_hdlr, _1, _2, _3, logger));
    }

    void signal_handler_manager::handle
      (int sig_num, siginfo_t* info, void* context) const
    {
      boost::mutex::scoped_lock const _ (_handler_mutex);
      BOOST_FOREACH ( const boost::function<void (int, siginfo_t*, void*)>& fun
                    , _handlers.find (sig_num)->second
                    )
      {
        fun (sig_num, info, context);
      }
    }
  }
}
