// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/Logger.hpp>

#include <functional>

#include <mutex>
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

      void handle (int sig_num, siginfo_t* info, void* context) const;

    private:
      friend struct scoped_signal_handler;

      mutable std::mutex _handler_mutex;
      using functions = std::list<std::function<void (int, siginfo_t*, void*)>>;
      std::map<int, std::pair<struct sigaction, functions>> _handlers;
    };

    struct scoped_signal_handler
    {
      scoped_signal_handler ( signal_handler_manager&
                            , int sig_num
                            , std::function<void (int, siginfo_t*, void*)>
                            );
      ~scoped_signal_handler();

    private:
      signal_handler_manager& _manager;
      int _sig_num;
      signal_handler_manager::functions::iterator _it;
    };

    struct scoped_log_backtrace_and_exit_for_critical_errors
    {
      scoped_log_backtrace_and_exit_for_critical_errors
        (signal_handler_manager&, fhg::log::Logger&);

    private:
      std::function<void (int, siginfo_t*, void*)> const _handler;
      scoped_signal_handler const _segv;
      scoped_signal_handler const _bus;
      scoped_signal_handler const _abrt;
      scoped_signal_handler const _fpe;
    };
  }
}
