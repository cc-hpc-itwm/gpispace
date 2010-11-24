#ifndef GPI_SPACE_CONFIG_NODE_CONFIG_HPP
#define GPI_SPACE_CONFIG_NODE_CONFIG_HPP 1

#include <inttypes.h>
#include <limits>
#include <gpi-space/config/logging.hpp>

namespace gpi_space
{
  namespace config
  {
#if defined(PATH_MAX)
#  define MAXPATHLEN PATH_MAX
#else
#  define MAXPATHLEN 4096
#endif
    static const std::size_t MAX_PATH_LEN = MAXPATHLEN;

    namespace api
    {
      static const int UNIX_STREAM = 0x01;
    }

    struct node_config_t
    {
      uint32_t provide_api;
      uint64_t memory_size;
      char sockets_path[MAX_PATH_LEN];

      logging::config logging;
    };
  }
}

#endif
