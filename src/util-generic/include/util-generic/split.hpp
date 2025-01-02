// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iterator>
#include <list>
#include <string>
#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename T, typename U, typename C = std::list<U>>
      inline C split ( T const& x
                     , typename std::iterator_traits<typename T::const_iterator>::value_type const& s
                     )
    {
      C path;
      T key;

      for (typename T::const_iterator pos (x.begin()); pos != x.end(); ++pos)
      {
        if (*pos == s)
        {
          path.push_back (key);
          key.clear();
        }
        else
        {
          key.push_back (*pos);
        }
      }

      if (!key.empty())
      {
        path.push_back (key);
      }

      return path;
    }


    inline std::pair<std::string, std::string> split_string
      (std::string const& val, char sep)
    {
      std::string::size_type const pos (val.find (sep));

      return std::make_pair
        ( val.substr (0, pos)
        , pos != std::string::npos ? val.substr (pos + 1) : ""
        );
    }
  }
}
