// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-generic/hostname.hpp>

#include <boost/format.hpp>

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <climits>

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
          ( (boost::format ("Could not get hostname: %1%") % strerror (errno))
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
