// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/hostname.hpp>

#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstring>

#include <fmt/core.h>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace
    {
      std::string get_hostname()
      {
        char buf [HOST_NAME_MAX + 1];

        buf[HOST_NAME_MAX] = '\0';

        if (gethostname (buf, HOST_NAME_MAX) == 0)
        {
          return std::string (buf);
        }

        throw std::runtime_error
          { fmt::format ("Could not get hostname: {}", strerror (errno))
          };
      }
    }

    std::string const& hostname()
    {
      static std::string const h (get_hostname());

      return h;
    }
  }
}
