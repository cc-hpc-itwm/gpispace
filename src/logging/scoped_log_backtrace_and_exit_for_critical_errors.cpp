// Copyright (C) 2014-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/signal_handler_manager.hpp>

#include <gspc/logging/message.hpp>
#include <gspc/logging/stream_emitter.hpp>

#include <gspc/util/backtracing_exception.hpp>

#include <functional>
#include <ios>
#include <sstream>

#include <signal.h>
#include <stdio.h>

namespace gspc::util
{
  namespace
  {
    [[noreturn]] void crit_err_hdlr
      ( int sig_num
      , siginfo_t* info
      , void* context
      , gspc::logging::stream_emitter& logger
      )
    {
      auto* mcontext (reinterpret_cast<sigcontext*> (
                              &(static_cast<ucontext_t*> (context)->uc_mcontext)
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

      std::ostringstream log_message;
      log_message << "received signal "
                  << sig_num << " (" << strsignal (sig_num) << "),"
                  << " address is " << static_cast<void*> (info->si_addr)
                  << " from " << std::hex << caller_address;

      logger.emit ( gspc::util::make_backtrace (log_message.str())
                  , gspc::logging::legacy::category_level_error
                  );

      _exit (EXIT_FAILURE);
    }
  }

  scoped_log_backtrace_and_exit_for_critical_errors::
    scoped_log_backtrace_and_exit_for_critical_errors
      ( signal_handler_manager& manager
      , gspc::logging::stream_emitter& logger
      )
    : _handler ( std::bind
                   ( &crit_err_hdlr
                   , std::placeholders::_1
                   , std::placeholders::_2
                   , std::placeholders::_3
                   , std::ref (logger)
                   )
               )
    , _segv (manager, SIGSEGV, _handler)
    , _bus (manager, SIGBUS, _handler)
    , _abrt (manager, SIGABRT, _handler)
    , _fpe (manager, SIGFPE, _handler)
  {}
}