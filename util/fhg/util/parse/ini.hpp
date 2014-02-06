// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_PARSE_INI_HPP
#define FHG_UTIL_PARSE_INI_HPP

#include <fhg/util/parse/position.hpp>

#include <list>
#include <map>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      // ini -> section*
      // section -> header body
      // header -> '[' label {sublabel} ']'
      // label -> STRING
      // sublabel -> '"' STRING '"'
      // body -> key_value*
      // key_value -> key '=' value
      // key -> STRING
      // value -> '"' STRING '"' | STRING_TO_END_OF_LINE

      // lines that start with \s*; or \s*# are ignored

      std::list<std::pair<std::string, std::string> > ini (position&);

      std::list<std::pair<std::string, std::string> >
        ini_from_string (std::string const&);

      std::map<std::string, std::string> ini_map (std::string const&);
    }
  }
}

#endif
