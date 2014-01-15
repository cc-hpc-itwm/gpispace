// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_SIGNAL_HANDLER_MANAGER_HPP
#define FHG_UTIL_SIGNAL_HANDLER_MANAGER_HPP

#include <fhglog/Logger.hpp>

#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <list>
#include <map>

#include <signal.h>

namespace fhg
{
  namespace util
  {
    struct signal_handler_manager
    {
      signal_handler_manager();
      ~signal_handler_manager();

      void add (int sig_num, boost::function<void (int, siginfo_t*, void*)> fun);

      //! \note handles SEGV, BUS, ABRT, FPE
      void add_log_backtrace_and_exit_for_critical_errors (fhg::log::Logger::ptr_t);

      void handle (int sig_num, siginfo_t* info, void* context) const;

      mutable boost::mutex _handler_mutex;
      std::map<int, std::list<boost::function<void (int, siginfo_t*, void*)> > >
        _handlers;
    };
  }
}

#endif
