#ifndef GPI_SPACE_CONFIG_PARSER_HPP
#define GPI_SPACE_CONFIG_PARSER_HPP 1

#include <gpi-space/config/node_config.hpp>

#include <iostream>

namespace gpi_space
{
  std::ostream & operator <<(std::ostream & os, config const & nc);
  std::istream & operator >>(std::istream & is, config & nc);
}

#include "node_config_io.tpp"

#endif
