#ifndef GPI_SPACE_CONFIG_PARSER_HPP
#define GPI_SPACE_CONFIG_PARSER_HPP 1

#include <fhg/util/ini-parser.hpp>
#include <fhg/util/ini-parser-helper.hpp>

#include <map>
#include <string>

namespace gpi_space
{
namespace parser
{
  typedef
  fhg::util::ini::parser::flat_map_parser_t < std::map< std::string
                                                      , std::string
                                                      >
                                            > config_parser_t;
  using ::fhg::util::ini::parse;
}
}

#endif
