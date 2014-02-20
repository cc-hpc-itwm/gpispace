#ifndef FHG_UTIL_URL_IO_HPP
#define FHG_UTIL_URL_IO_HPP

#include <string>
#include <ostream>

#include <gpi-space/pc/url.hpp>
#include <fhg/util/first_then.hpp>

#include <boost/foreach.hpp>

namespace gpi
{
  namespace pc
  {
    inline std::ostream & operator<< (std::ostream & os, const url_t &url)
    {
      os << url.type () << "://" << url.path ();

      fhg::util::first_then<std::string> const sep ("?", "&");

      BOOST_FOREACH
        ( std::pair<std::string BOOST_PP_COMMA() std::string> const& kv
        , url.args()
        )
      {
        os << sep << kv.first << '=' << kv.second;
      }

      return os;
    }
  }
}

#endif
