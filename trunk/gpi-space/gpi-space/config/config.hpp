#ifndef GPI_SPACE_CONFIG_CONFIG_HPP
#define GPI_SPACE_CONFIG_CONFIG_HPP 1

#include <gpi-space/config/config-data.hpp>
#include <gpi-space/config/node.hpp>
#include <gpi-space/config/logging.hpp>
#include <gpi-space/config/gpi.hpp>

namespace gpi_space
{
  struct config
  {
    node::config node;
    gpi::config gpi;
    logging::config logging;
  };
}

#endif
