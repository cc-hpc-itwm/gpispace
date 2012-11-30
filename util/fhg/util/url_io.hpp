#ifndef FHG_UTIL_URL_IO_HPP
#define FHG_UTIL_URL_IO_HPP

#include <string>
#include <ostream>
#include <istream>

#include <fhg/util/url.hpp>

namespace fhg
{
  namespace util
  {
    inline std::ostream & operator<< (std::ostream & os, const url_t &url)
    {
      os << url.type () << "://" << url.path ();
      if (not url.args ().empty ())
      {
        bool first = true;

        url_t::arg_map_t::const_iterator it = url.args ().begin ();
        url_t::arg_map_t::const_iterator end = url.args ().end ();

        while (it != end)
        {
          if (first)
          {
            os << "?";
            first = false;
          }
          else
          {
            os << "&";
          }

          os << it->first << "=" << it->second;
          ++it;
        }
      }
      return os;
    }

    inline std::istream & operator>> (std::istream &is, url_t & url)
    {
      std::string s;
      is >> s;
      url = url_t (s);
      return is;
    }
  }
}

#endif
