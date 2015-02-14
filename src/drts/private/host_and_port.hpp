#pragma once

#include <string>

namespace gspc
{
  struct host_and_port_type
  {
    host_and_port_type (std::string const& host, unsigned short const port)
      : host (host)
      , port (port)
    {}

    std::string const host;
    unsigned short const port;
  };
}
