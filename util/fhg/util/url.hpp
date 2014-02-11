#ifndef FHG_UTIL_URL_HPP
#define FHG_UTIL_URL_HPP

#include <string>
#include <map>

namespace fhg
{
  namespace util
  {
    class url_t
    {
    public:
      typedef std::map<std::string, std::string> arg_map_t;

      url_t ()
      {}

      // url -> protocoll { '://' { host_path } { '?' { parameter_list }}}
      // protocoll -> identifier
      // host_path -> host_and_port { '/' path }
      // host_and_port -> host { ':' port }
      // host -> ip | identifier_with_dot | '*'
      // ip -> UINT8 '.' UINT8 '.' UINT8 '.' UINT8
      // port -> UINT16 | '*'
      // path -> identifier { '/' path }
      // parameter_list -> parameter { '&' parameter_list }
      // parameter -> key '=' value // throw on duplicate key
      // key -> identifier
      // value -> [a-zA-Z_0-9]+
      // identifier -> [a-zA-Z_][a-zA-Z_0-9]*
      // identifier_with_dot -> [a-zA-Z_][a-zA-Z_0-9.]*
      explicit
      url_t (std::string const &);

      std::string const & type () const { return m_type; }
      std::string const & path () const { return m_path; }
      arg_map_t const &   args () const { return m_args; }

      void type (std::string const &s)
      {
        m_type = s;
      }

      void path (std::string const &s)
      {
        m_path = s;
      }

      void set (std::string const &k, std::string const &v)
      {
        m_args [k] = v;
      }

      std::string get (std::string const &k, std::string const &dflt="") const
      {
        arg_map_t::const_iterator it = m_args.find (k);
        if (it != m_args.end ())
          return it->second;
        else
          return dflt;
      }
    private:
      std::string m_type;
      std::string m_path;
      arg_map_t   m_args;
    };
  }
}

#endif
