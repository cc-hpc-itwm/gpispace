#ifndef GPI_SPACE_CONFIG_NODE_CONFIG_HPP
#define GPI_SPACE_CONFIG_NODE_CONFIG_HPP 1

#include <inttypes.h>
#include <string.h> // memset
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
        , mode (0600)
      {
        memset (sockets_path, 0, MAX_PATH_LEN);
      }

      bool daemonize;
      char sockets_path[MAX_PATH_LEN];
      int mode;
    };
  }

  struct config
  {
    node::config node;
    gpi::config gpi;
    logging::config logging;
  };
}

#endif
