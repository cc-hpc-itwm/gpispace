// -*- mode: c++; -*-
#include "config_io.hpp"

#include <fhg/util/ini-parser.hpp>

#include <cstring>

namespace gpi_space
{
  // TODO: ini-style serialization
  std::ostream & operator <<(std::ostream & os, gpi::config const & gc)
  {
    os << "[gpi]"      << std::endl;
    os << "  memory_size = " << gc.memory_size << std::endl;
    os << "  mtu = " << gc.mtu << std::endl;
    os << "  port = " << gc.port << std::endl;
    os << "  network_type = " << gc.network_type << std::endl;
    os << "  processes = " << gc.processes << std::endl;
    os << "  socket_path = " << gc.socket_path << std::endl;
    os << "  socket_name = " << gc.socket_name << std::endl;
    os << "  mode = " << std::oct << gc.mode << std::dec << std::endl;
    return os;
  }

  std::ostream & operator <<(std::ostream & os, logging::config const & lc)
  {
    return os;
  }

  std::ostream & operator <<(std::ostream & os, node::config const & nc)
  {
    os << "[node]" << std::endl;
    os << "  daemonize = " << nc.daemonize << std::endl;
    return os;
  }

  std::ostream & operator <<(std::ostream & os, gpi_space::config const & c)
  {
    os << "; GPI-Space config file" << std::endl;
    os << ";" << std::endl;
    os << c.node << std::endl;
    os << c.gpi << std::endl;;
    os << c.logging << std::endl;
    return os;
  }

  std::istream & operator >>(std::istream & is, gpi_space::config & c)
  {
    return is;
  }
}
