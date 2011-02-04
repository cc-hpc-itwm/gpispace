#ifndef GPI_SPACE_CONFIG_NODE_HPP
#define GPI_SPACE_CONFIG_NODE_HPP 1

#include <inttypes.h>
#include <string.h> // memset
#include <fhg/util/read_bool.hpp>
#include <gpi-space/config/config-data.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>

namespace gpi_space
{
  namespace node
  {
    struct config
    {
      config ()
        : daemonize (false)
        , mode (0700)
      {
        memset (socket_path, 0, gpi_space::MAX_PATH_LEN);
      }

      template <typename Mapping>
      void load (Mapping const & m)
      {
        daemonize = fhg::util::read_bool
          (m.get("node.daemonize", "false"));
      }

      bool daemonize;
    };
  }
}

#endif
