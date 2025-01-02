// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

#include <cstdlib>
#include <string>

namespace fhg
{
  namespace util
  {
    [[deprecated ("use std::getenv from <cstdlib> instead, will be removed after 2025/12/31")]]
    inline ::boost::optional<const char*> getenv (const char* env_var)
    {
      const char *val (std::getenv (env_var));

      if (val)
      {
        return val;
      }

      return ::boost::none;
    }
  }
}
