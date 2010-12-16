#ifndef GPI_SPACE_CONFIG_NODE_HPP
#define GPI_SPACE_CONFIG_NODE_HPP 1

#include <inttypes.h>
#include <string.h> // memset
#include <gpi-space/config/config-data.hpp>
#include <boost/lexical_cast.hpp>

namespace gpi_space
{
  namespace node
  {
    struct config
    {
      config ()
        : daemonize (false)
        , mode (0600)
      {
        memset (socket_path, 0, gpi_space::MAX_PATH_LEN);
      }

      template <typename Mapping>
      void load (Mapping const & m)
      {
        snprintf(socket_path, gpi_space::MAX_PATH_LEN, "%s", m.get("node.socket_path", "/var/tmp").c_str());
      }

      bool daemonize;
      char socket_path[gpi_space::MAX_PATH_LEN];
      int mode;
    };
  }
}

#endif
