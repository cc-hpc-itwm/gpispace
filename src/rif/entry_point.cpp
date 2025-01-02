// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <rif/entry_point.hpp>

#include <sstream>
#include <stdexcept>
#include <tuple>

namespace fhg
{
  namespace rif
  {
    entry_point::entry_point() = default;

    entry_point::entry_point
      ( std::string const& hostname_
      , unsigned short port_
      , pid_t pid_
      )
        : hostname (hostname_)
        , port (port_)
        , pid (pid_)
    {}

    entry_point::entry_point (std::string const& input)
    {
      std::istringstream iss (input);
      if (!(iss >> hostname >> port >> pid))
      {
        throw std::runtime_error
          ("parse error: expected 'host port pid': got '" + input + "'");
      }
    }

    std::ostream& entry_point::operator() (std::ostream& os) const
    {
      return os << hostname << ' ' << port << ' ' << pid;
    }

    bool entry_point::operator== (entry_point const& other) const
    {
#define ESSENCE(x) std::tie ((x).hostname, (x).port, (x).pid)
      return ESSENCE (*this) == ESSENCE (other);
#undef ESSENCE
    }
  }
}
