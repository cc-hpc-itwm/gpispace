#ifndef GPI_SPACE_CONFIG_IO_HPP
#define GPI_SPACE_CONFIG_IO_HPP 1

#include <gpi-space/config/config.hpp>

#include <iostream>

namespace gpi_space
{
  std::ostream & operator <<(std::ostream & os, gpi_space::config const & );
  std::istream & operator >>(std::istream & is, gpi_space::config & c);
}

#include "config_io.tpp"

#endif
