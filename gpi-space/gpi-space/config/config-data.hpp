#ifndef GPI_SPACE_CONFIG_CONFIG_DATA_HPP
#define GPI_SPACE_CONFIG_CONFIG_DATA_HPP 1

#include <limits>
#include <string>
#include <boost/unordered_map.hpp>

namespace gpi_space
{
  static const std::size_t MAX_HOST_LEN = 256;

#if defined(PATH_MAX)
  static const std::size_t MAX_PATH_LEN = PATH_MAX;
#else
  static const std::size_t MAX_PATH_LEN = 4096;
#endif

  typedef boost::unordered_map<std::string, std::string> config_data_t;
}

#endif
