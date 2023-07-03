// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/hostname.hpp>

#include <boost/format.hpp>

#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstring>

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
          ( (::boost::format ("Could not get hostname: %1%") % strerror (errno))
          . str()
          );
      }
    }

    std::string const& hostname()
    {
      static std::string const h (get_hostname());

      return h;
    }
  }
}
