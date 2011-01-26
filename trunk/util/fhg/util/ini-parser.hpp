#ifndef FHG_UTIL_INI_PARSER_HPP
#define FHG_UTIL_INI_PARSER_HPP 1

#include <istream>
#include <string>

#include <boost/function.hpp>
#include <boost/optional.hpp>

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
  struct key_desc_t
  {
    key_desc_t ()
    {}
    key_desc_t ( std::string const & a_sec
               , std::string const & a_key
               )
      : sec (a_sec)
      , key (a_key)
    {}
    key_desc_t ( std::string const & a_sec
               , std::string const & a_key
               , std::string const & a_id
               )
      : sec (a_sec)
      , key (a_key)
      , id  (a_id)
    {}

    std::string sec;
    std::string key;
    boost::optional<std::string> id;
  };

  typedef boost::function<int ( key_desc_t const & desc
                              , std::string const & value
                              ) > entry_handler_t;

  void parse (std::istream & is, entry_handler_t handler);
  void parse ( std::istream & is
             , std::string const & stream_descriptor
             , entry_handler_t handler
             );
  void parse (std::string const & path, entry_handler_t handler);
}
}
}

#include "ini-parser.tcc"

#endif
