// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <logging/stream_emitter.hpp>

#include <functional>

#include <list>
#include <map>
#include <mutex>

#include <signal.h>

namespace fhg
{
  namespace util
  {
    struct signal_handler_manager
    {
      signal_handler_manager();
      ~signal_handler_manager();
      signal_handler_manager (signal_handler_manager const&) = delete;
      signal_handler_manager (signal_handler_manager&&) = delete;
      signal_handler_manager& operator= (signal_handler_manager const&) = delete;
      signal_handler_manager& operator= (signal_handler_manager&&) = delete;

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
      scoped_signal_handler (scoped_signal_handler const&) = delete;
      scoped_signal_handler (scoped_signal_handler&&) = delete;
      scoped_signal_handler& operator= (scoped_signal_handler const&) = delete;
      scoped_signal_handler& operator= (scoped_signal_handler&&) = delete;

    private:
      signal_handler_manager& _manager;
      int _sig_num;
      signal_handler_manager::functions::iterator _it;
    };

    struct scoped_log_backtrace_and_exit_for_critical_errors
    {
      scoped_log_backtrace_and_exit_for_critical_errors
        (signal_handler_manager&, fhg::logging::stream_emitter&);

    private:
      std::function<void (int, siginfo_t*, void*)> const _handler;
      scoped_signal_handler const _segv;
      scoped_signal_handler const _bus;
      scoped_signal_handler const _abrt;
      scoped_signal_handler const _fpe;
    };

    //! Provides stop() and captures SIGTERM, SIGINT to stop execution.
    struct Execution
    {
      Execution (signal_handler_manager&);

      //! blocks until stop() is called or SIGTERM, SIGINT arrive
      void wait();

      //! triggers wait()
      void stop();

    private:
      bool _terminated {false};
      std::mutex _guard_terminated;
      std::condition_variable _was_terminated;
      void notify (int, siginfo_t*, void*);
      scoped_signal_handler const _term;
      scoped_signal_handler const _int;
    };
  }
}
