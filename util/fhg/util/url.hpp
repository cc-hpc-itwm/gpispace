#ifndef FHG_UTIL_URL_HPP
#define FHG_UTIL_URL_HPP

#include <string>
#include <map>

#include <iostream>

#include <fhg/util/split.hpp>

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
      url_t (std::string const &u)
      {
        *this = parse (u);
      }

      url_t (const url_t &o)
        : m_type (o.m_type)
        , m_path (o.m_path)
        , m_args (o.m_args)
      {}

      url_t & operator= (url_t const & o)
      {
        if (this != &o)
        {
          m_type = o.m_type;
          m_path = o.m_path;
          m_args = o.m_args;
        }
        return *this;
      }

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

      url_t & del (std::string const &k)
      {
        m_args.erase (k);
        return *this;
      }
    private:
      static url_t parse (std::string const &input)
      {
        url_t url;

        // type, rest = split ://
        // path, args = split on ?
        // arg_k, arg_v = split = (split & args)

        typedef std::pair<std::string, std::string> split_p;
        split_p p;

        p = split (input, "://");

        if (p.first.empty ())
        {
          throw std::invalid_argument ("no type found in url: " + input);
        }

        url.type (p.first);

        if (p.second.find ("?") != std::string::npos)
        {
          p = split (p.second, "?");
          url.path (p.first);
          std::string args = p.second;

          while (args.size ())
          {
            p = split (args, "&");

            args = p.second;

            if (p.first.empty ())
              continue;
            p = split (p.first, "=");
            if (p.first.empty ())
            {
              throw std::invalid_argument ("empty parameter in url: " + input);
            }

            url.set (p.first, p.second);
          }
        }
        else if (p.second.find ("&") == std::string::npos)
        {
          url.path (p.second);
        }
        else
        {
          throw std::invalid_argument ("malformed url: & not allowed in path");
        }

        return url;
      }

      std::string m_type;
      std::string m_path;
      arg_map_t   m_args;
    };
  }
}

#endif
