#ifndef GPI_SPACE_CONFIG_NODE_CONFIG_HPP
#define GPI_SPACE_CONFIG_NODE_CONFIG_HPP 1

#include <inttypes.h>
#include <limits>
#include <gpi-space/config/logging.hpp>
#include <gpi-space/config/gpi.hpp>

namespace gpi_space
{
  namespace node
  {
#if defined(PATH_MAX)
#  define MAXPATHLEN PATH_MAX
#else
#  define MAXPATHLEN 4096
#endif
    static const std::size_t MAX_PATH_LEN = MAXPATHLEN;

    struct config
    {
      config ()
        : daemonize (false)
      {}

      bool daemonize;
      char sockets_path[MAX_PATH_LEN];

      logging::config logging;
      gpi::config gpi;
    };
  }
}

#endif
