// -*- mode: c++; -*-
#include "node_config_io.hpp"
#include <fhg/util/ini-parser.hpp>

#include <cstring>

namespace gpi_space
{
  namespace node
  {
    // TODO: ini-style serialization


    std::ostream & operator <<(std::ostream & os, gpi::config const & gc)
    {
      os << "[gpi]"      << std::endl;
      os << "memory_size = " << gc.memory_size << std::endl;
      os << "mtu = " << "default" << std::endl;
      os << "port = " << "default" << std::endl;
      os << "network_type = " << "default" << std::endl;
      os << "processes = " << "default" << std::endl;
      return os;
    }

    std::ostream & operator <<(std::ostream & os, logging::config const & lc)
    {
      return os;
    }

    std::ostream & operator <<(std::ostream & os, node::config const & nc)
    {
      return os;
    }

    std::istream & operator >>(std::istream & is, node::config & nc)
    {
      nc.gpi.memory_size = (1<<20);
      strncpy (nc.sockets_path, "/tmp/test.cfg", MAX_PATH_LEN);
      return is;
    }
  }
}
