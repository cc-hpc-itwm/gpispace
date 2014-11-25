#ifndef DRTS_PRIVATE_HOST_AND_PORT_HPP
#define DRTS_PRIVATE_HOST_AND_PORT_HPP

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

#endif
