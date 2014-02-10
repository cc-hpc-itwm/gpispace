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

      explicit
      url_t (std::string const &);

      std::string const & type () const { return m_type; }
      std::string const & path () const { return m_path; }
      arg_map_t const &   args () const { return m_args; }

      url_t & type (std::string const &s)
      {
        m_type = s;
        return *this;
      }

      url_t & path (std::string const &s)
      {
        m_path = s;
        return *this;
      }

      url_t & set (std::string const &k, std::string const &v)
      {
        m_args [k] = v;
        return *this;
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
