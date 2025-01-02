// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <logging/endpoint.hpp>
#include <logging/stream_receiver.hpp>

#include <vector>

namespace fhg
{
  namespace logging
  {
    struct stdout_sink : public stream_receiver
    {
    public:
      //! \todo Formatter.
      stdout_sink();
      stdout_sink (std::vector<endpoint> emitters);
    };
  }
}
