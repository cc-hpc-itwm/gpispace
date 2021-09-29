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
