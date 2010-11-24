#ifndef FHG_UTIL_INI_PARSER_HPP
#define FHG_UTIL_INI_PARSER_HPP 1

#include <istream>
#include <string>

#include <boost/function.hpp>

namespace fhg
{
  namespace util
  {
    namespace ini
    {
      typedef boost::function<int ( std::string const & section
                                  , std::string const * section_id // 3 states: set, empty, notset (=NULL)
                                  , std::string const & key
                                  , std::string const & value
                                  ) > entry_handler_t;

      void parse (std::istream & is, entry_handler_t handler);
      void parse (std::string const & path, entry_handler_t handler);
    }
  }
}

#include "ini-parser.tcc"

#endif
