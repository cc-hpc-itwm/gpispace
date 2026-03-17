// Copyright (C) 2019-2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/endpoint.hpp>
#include <gspc/logging/stream_receiver.hpp>

#include <vector>


  namespace gspc::logging
  {
    struct stdout_sink : public stream_receiver
    {
    public:
      //! \todo Formatter.
      stdout_sink();
      stdout_sink (std::vector<endpoint> emitters);
    };
  }
