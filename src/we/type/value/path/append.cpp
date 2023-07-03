// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/path/append.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        append::append ( std::list<std::string>& path
                       , std::string const& key
                       )
          : _path (path)
        {
          _path.push_back (key);
        }
        append::~append()
        {
          _path.pop_back();
        }
        append::operator std::list<std::string>&() const
        {
          return _path;
        }
      }
    }
  }
}
