#pragma once

#include <string>
#include <ostream>

#include <gpi-space/pc/url.hpp>
#include <util-generic/first_then.hpp>

namespace gpi
{
  namespace pc
  {
    inline std::ostream & operator<< (std::ostream & os, const url_t &url)
    {
      os << url.type () << "://" << url.path ();

      fhg::util::first_then<std::string> const sep ("?", "&");

      for (std::pair<std::string, std::string> const& kv : url.args())
      {
        os << sep << kv.first << '=' << kv.second;
      }

      return os;
    }
  }
}
