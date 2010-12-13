// -*- mode: c++; -*-
#include "node_config_io.hpp"

#include <cstring>

namespace gpi_space
{
  namespace config
  {
    std::ostream & operator <<(std::ostream & os, node_config_t const & nc)
    {
      os << nc.gpi.memory_size;
      return os;
    }

    std::istream & operator >>(std::istream & is, node_config_t & nc)
    {
      nc.provide_api |= api::UNIX_STREAM;
      nc.gpi.memory_size = (1<<20);
      strncpy (nc.sockets_path, "/tmp/test.cfg", MAX_PATH_LEN);
      return is;
    }
  }
}
