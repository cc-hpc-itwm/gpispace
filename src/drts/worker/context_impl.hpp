// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/worker/context.hpp>

#include <logging/stream_emitter.hpp>

#include <util-generic/ostream/redirect.hpp>

#include <set>
#include <string>

#include <functional>

namespace drts
{
  namespace worker
  {
    class context_constructor
    {
    public:
      context_constructor ( std::string const& worker_name
                          , std::set<std::string> const& workers
                          , fhg::logging::stream_emitter& logger
                          );
      context::implementation* _;
    };

    class context::implementation
    {
    public:
      implementation ( std::string const& worker_name
                     , std::set<std::string> const& workers
                     , fhg::logging::stream_emitter& logger
                     );

      std::string const& worker_name() const;

      std::set<std::string> const& workers () const;
      std::string worker_to_hostname (std::string const&) const;

      void set_module_call_do_cancel (std::function<void()> fun);
      void module_call_do_cancel();

      void execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
        ( std::function<void()> fun
        , std::function<void()> on_cancel
        , std::function<void (int)> on_signal
        , std::function<void (int)> on_exit
        );

      void log (char const* category, std::string const& message) const;

    private:
      std::string _worker_name;
      std::set<std::string> _workers;
      std::function<void()> _module_call_do_cancel;
      bool _cancelled {false};
      fhg::logging::stream_emitter& _logger;
    };

    class redirect_output : public fhg::util::ostream::redirect
    {
    public:
      redirect_output ( drts::worker::context const* const context
                      , char const* const category
                      , std::ostream& os
                      )
        : redirect
          ( os
          , [context, category] (std::string const& line)
            {
              context->_->log (category, line);
            }
          )
      {}
    };
  }
}
