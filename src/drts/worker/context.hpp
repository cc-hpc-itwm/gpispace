// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/worker/context_fwd.hpp>

#include <functional>
#include <set>
#include <stdexcept>
#include <string>

namespace drts
{
  namespace worker
  {
    class redirect_output;
    class GSPC_DLLEXPORT context_constructor;

    [[noreturn]] GSPC_DLLEXPORT void throw_cancelled();
    [[noreturn]] GSPC_DLLEXPORT void on_signal_unexpected (int);
    [[noreturn]] GSPC_DLLEXPORT void on_exit_unexpected (int);

    class GSPC_DLLEXPORT context
    {
    private:
      friend class redirect_output;
      friend class context_constructor;
      class implementation;
      implementation* _;

    public:
      context (context_constructor);
      ~context();
      context (context const&) = delete;
      context (context&&) = delete;
      context& operator= (context const&) = delete;
      context& operator= (context&&) = delete;

      std::string const& worker_name() const;

      std::set<std::string> const& workers() const;
      std::string worker_to_hostname (std::string const&) const;

      void module_call_do_cancel() const;

      void execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
        ( std::function<void()> fun
        , std::function<void()> on_cancel
        , std::function<void (int)> on_signal
        , std::function<void (int)> on_exit
        );

      void execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
        ( std::function<void()> fun
        , std::function<void()> on_cancel
        )
      {
        execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( fun
          , on_cancel
          , &on_signal_unexpected
          , &on_exit_unexpected
          );
      }

      struct GSPC_DLLEXPORT cancelled : public std::exception {};

      void execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN (std::function<void()> fun)
      {
        execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( fun
          , &throw_cancelled
          );
      }
    };
  }
}
