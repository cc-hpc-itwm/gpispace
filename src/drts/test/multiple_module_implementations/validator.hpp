// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/worker/context.hpp>

#include <fmt/core.h>
#include <stdexcept>
#include <string>

namespace
{
  void validate_implementation
    ( drts::worker::context const* context
    , std::string const& expected_implementation
    )
  {
    auto const workers (context->workers());
    if (workers.size() != 1)
    {
      throw std::runtime_error
        { fmt::format ("Unexpected number of workers: {}!", workers.size())
        };
    }

    auto const worker (context->worker_name());
    if (worker.find (expected_implementation) == std::string::npos)
    {
      throw std::runtime_error
        { fmt::format
            ( "Worker {}: missing capability {}!"
            , worker
            , expected_implementation
            )
        };
    }
  }
}
