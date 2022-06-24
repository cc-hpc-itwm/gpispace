// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <fhg/util/signal_handler_manager.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/backtracing_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <boost/range/adaptor/map.hpp>

#include <ios>
#include <stdexcept>

#include <stdio.h>

namespace fhg
{
  namespace util
  {
    namespace
    {
      std::mutex GLOBAL_manager_mutex;
      signal_handler_manager* GLOBAL_manager;

      void signal_handler (int sig_num, siginfo_t* info, void* context)
      {
        std::lock_guard<std::mutex> const _ (GLOBAL_manager_mutex);
        if (GLOBAL_manager)
        {
          GLOBAL_manager->handle (sig_num, info, context);
        }
      }
    }

    signal_handler_manager::signal_handler_manager()
    {
      std::lock_guard<std::mutex> const _ (GLOBAL_manager_mutex);
      fhg_assert (!GLOBAL_manager);
      GLOBAL_manager = this;
    }

    signal_handler_manager::~signal_handler_manager()
    {
      std::lock_guard<std::mutex> const _ (GLOBAL_manager_mutex);
      GLOBAL_manager = nullptr;

      for (decltype (_handlers)::value_type const& handler : _handlers)
      {
        util::syscall::sigaction (handler.first, &handler.second.first, nullptr);
      }
    }

    void signal_handler_manager::handle
      (int sig_num, siginfo_t* info, void* context) const
    {
      std::lock_guard<std::mutex> const _ (_handler_mutex);
      for ( std::function<void (int, siginfo_t*, void*)> const& fun
          : _handlers.find (sig_num)->second.second
          )
      {
        fun (sig_num, info, context);
      }
    }

    scoped_signal_handler::scoped_signal_handler
        ( signal_handler_manager& manager
        , int sig_num
        , std::function<void (int, siginfo_t*, void*)> fun
        )
      : _manager (manager)
      , _sig_num (sig_num)
    {
      std::lock_guard<std::mutex> const _ (_manager._handler_mutex);

      if (_manager._handlers.find (_sig_num) == _manager._handlers.end())
      {
        struct sigaction sigact;
        memset (&sigact, 0, sizeof (sigact));
        sigact.sa_sigaction = signal_handler;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        util::syscall::sigaction
          (_sig_num, &sigact, &_manager._handlers[_sig_num].first);
      }

      _it = _manager._handlers[_sig_num].second.insert
        (_manager._handlers[_sig_num].second.end(), fun);
    }
    scoped_signal_handler::~scoped_signal_handler()
    {
      std::lock_guard<std::mutex> const _ (_manager._handler_mutex);

      _manager._handlers.find (_sig_num)->second.second.erase (_it);
    }

    namespace
    {
      [[noreturn]] void crit_err_hdlr
        ( int sig_num, siginfo_t* info, void* context
        , fhg::logging::stream_emitter& logger
        )
      {
        auto* mcontext (static_cast<sigcontext*> (static_cast<void*>
                               (&(static_cast<ucontext_t*> (context)->uc_mcontext))
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

        logger.emit ( fhg::util::make_backtrace (log_message.str())
                    , fhg::logging::legacy::category_level_error
                    );

        _exit (EXIT_FAILURE);
      }
    }

    scoped_log_backtrace_and_exit_for_critical_errors::
      scoped_log_backtrace_and_exit_for_critical_errors
        (signal_handler_manager& manager, fhg::logging::stream_emitter& logger)
      : _handler ( std::bind ( &crit_err_hdlr
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

    Execution::Execution
      (signal_handler_manager& manager)
        : _term (manager, SIGTERM, fhg::util::bind_this (this, &Execution::notify))
        , _int (manager, SIGINT, fhg::util::bind_this (this, &Execution::notify))
    {}

    void Execution::notify (int, siginfo_t*, void*)
    {
      return stop();
    }

    void Execution::stop()
    {
      std::lock_guard<std::mutex> const lock (_guard_terminated);

      _terminated = true;

      _was_terminated.notify_all();
    }

    void Execution::wait()
    {
      std::unique_lock<std::mutex> lock (_guard_terminated);

      _was_terminated.wait (lock, [&] { return _terminated; });
    }
  }
}
