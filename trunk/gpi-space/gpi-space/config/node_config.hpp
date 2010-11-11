#ifndef GPI_SPACE_CONFIG_NODE_CONFIG_HPP
#define GPI_SPACE_CONFIG_NODE_CONFIG_HPP 1

#include <inttypes.h>

namespace gpi_space
{
  namespace config
  {
    static const int MAX_PATH_LEN = 2048;

    namespace api
    {
      static const int UNIX_STREAM = 0x01;
    }

    struct node_config_t
    {
      uint32_t provide_api;
      uint64_t memory_size;
      char sockets_path[MAX_PATH_LEN];
    };
  }
}

#endif
