// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/path/split.hpp>

#include <util-generic/split.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        std::list<std::string> split (std::string const& path)
        {
          return fhg::util::split<std::string, std::string> (path, '.');
        }
      }
    }
  }
}
