#ifndef FHG_UTIL_INI_PARSER_EXCEPT_HPP
#define FHG_UTIL_INI_PARSER_EXCEPT_HPP 1

#include <stdexcept>

namespace fhg
{
namespace util
{
namespace ini
{
namespace exception
{
  struct parse_error : public std::runtime_error
  {
    parse_error (std::string const & msg)
      : std::runtime_error (msg)
    {}

    ~parse_error () throw () {}
  };
}
}
}
}

#endif
