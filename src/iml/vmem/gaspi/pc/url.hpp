#pragma once

#include <string>
#include <map>

#include <boost/optional.hpp>

namespace gpi
{
  namespace pc
  {
    class url_t
    {
    public:
      typedef std::map<std::string, std::string> arg_map_t;

      url_t (std::string const& type, std::string const& path)
        : m_type (type)
        , m_path (path)
      {}

      // url -> protocoll { '://' { host_path } { '?' { parameter_list }}}
      // protocoll -> identifier
      // host_path -> host_and_port { '/' path }
      // host_and_port -> host { ':' port }
      // host -> ip | identifier_with_dot_and_space | '*'
      // ip -> UINT8 '.' UINT8 '.' UINT8 '.' UINT8
      // port -> UINT16 | '*'
      // path -> identifier { '/' path }
      // parameter_list -> parameter { '&' parameter_list }
      // parameter -> key '=' value // throw on duplicate key
      // key -> identifier
      // value -> [a-zA-Z_0-9]+
      // identifier -> [a-zA-Z_][a-zA-Z_0-9]*
      // identifier_with_dot_and_space -> [a-zA-Z_][a-zA-Z_0-9. ]*
      explicit
      url_t (std::string const &);

      std::string const & type () const { return m_type; }
      std::string const & path () const { return m_path; }
      arg_map_t const &   args () const { return m_args; }

      void set (std::string const &k, std::string const &v)
      {
        m_args [k] = v;
      }

      boost::optional<std::string> get (std::string const&) const;
    private:
      std::string m_type;
      std::string m_path;
      arg_map_t   m_args;
    };
  }
}
