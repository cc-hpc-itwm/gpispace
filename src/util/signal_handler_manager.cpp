// Copyright (C) 2014-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/signal_handler_manager.hpp>

#include <gspc/assert.hpp>
#include <gspc/util/backtracing_exception.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>

#include <ios>
#include <stdexcept>
#include <cstring>

#include <stdio.h>


  namespace gspc::util
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
      gspc_assert (!GLOBAL_manager);
      GLOBAL_manager = this;
    }

    signal_handler_manager::~signal_handler_manager()
    {
      std::lock_guard<std::mutex> const _ (GLOBAL_manager_mutex);
      GLOBAL_manager = nullptr;

      for (decltype (_handlers)::value_type const& handler : _handlers)
      {
        gspc::util::syscall::sigaction (handler.first, &handler.second.first, nullptr);
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
        std::memset (&sigact, 0, sizeof (sigact));
        sigact.sa_sigaction = signal_handler;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        gspc::util::syscall::sigaction
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

    Execution::Execution
      (signal_handler_manager& manager)
        : _term (manager, SIGTERM, gspc::util::bind_this (this, &Execution::notify))
        , _int (manager, SIGINT, gspc::util::bind_this (this, &Execution::notify))
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
