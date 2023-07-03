// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace gspc
{
  struct host_and_port_type
  {
    host_and_port_type (std::string const& host_, unsigned short const port_)
      : host (host_)
      , port (port_)
    {}

    std::string const host;
    unsigned short const port;
  };
}
