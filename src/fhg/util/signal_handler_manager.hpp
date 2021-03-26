// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#pragma once

#include <logging/stream_emitter.hpp>

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
        (signal_handler_manager&, fhg::logging::stream_emitter&);

    private:
      std::function<void (int, siginfo_t*, void*)> const _handler;
      scoped_signal_handler const _segv;
      scoped_signal_handler const _bus;
      scoped_signal_handler const _abrt;
      scoped_signal_handler const _fpe;
    };
  }
}
