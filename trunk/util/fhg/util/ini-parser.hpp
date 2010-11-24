#ifndef FHG_UTIL_INI_PARSER_HPP
#define FHG_UTIL_INI_PARSER_HPP 1

#include <istream>
#include <string>

#include <boost/function.hpp>

/*
  simple ini-file parser with some additional features:

  %include <path>
     includes the specified path (relative paths are also possible)

     not yet implemented ;-(

  section-ids:

      [section]         no section id
         key = val
      [section ""]      empty section id
         key = val
      [section "id"]    section-id 'id'
         key = val

      this enables stuff like:

      [appender "console"]
         level = 1
         type  = stream
         sink  = stderr
      [appender "file"]
         level = 2
         type  = file
         sink  = /var/log/...
      [appender "net"]
         level = 1
         type = net
         sink = 127.0.0.1:1234
      [logger ""]
         level = 0
         appender = console,file,net

    in a flat view this can be represented as:

      appender.console.level = 1
      appender.console.type = stream

    and so on, section-ids can contain '.', so that
    stuff like

      logging.appender.console.level = 1

    can be represented as

      [logging "appender.console"]
        level = 1
*/

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
