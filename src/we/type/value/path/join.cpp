// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/path/join.hpp>

#include <util-generic/join.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        std::string join (std::list<std::string> const& path)
        {
          return fhg::util::join (path, '.').string();
        }
      }
    }
  }
}
