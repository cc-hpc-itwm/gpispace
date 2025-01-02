// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        GSPC_DLLEXPORT std::string join (std::list<std::string> const&);
      }
    }
  }
}
