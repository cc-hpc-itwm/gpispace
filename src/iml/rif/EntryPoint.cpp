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

#include <iml/rif/EntryPoint.hpp>

#include <util-generic/hash/combined_hash.hpp>

#include <sstream>
#include <stdexcept>
#include <tuple>

namespace iml
{
  namespace rif
  {
    EntryPoint::EntryPoint
        (std::string const& hostname_, unsigned short port_, pid_t pid_)
      : hostname (hostname_)
      , port (port_)
      , pid (pid_)
    {}

    EntryPoint::EntryPoint (std::string const& input)
    {
      std::istringstream iss (input);
      if (!(iss >> hostname >> port >> pid))
      {
        throw std::runtime_error
          ("parse error: expected 'host port pid': got '" + input + "'");
      }
    }
    std::ostream& operator<< (std::ostream& os, EntryPoint const& entry_point)
    {
      return os << entry_point.hostname
                << ' ' << entry_point.port
                << ' ' << entry_point.pid;
    }

    bool operator== (EntryPoint const& lhs, EntryPoint const& rhs)
    {
      return std::tie (lhs.hostname, lhs.port, lhs.pid)
        == std::tie (rhs.hostname, rhs.port, rhs.pid);
    }
  }
}

namespace std
{
  std::size_t hash<iml::rif::EntryPoint>::operator()
    (iml::rif::EntryPoint const& ep) const
  {
    return fhg::util::combined_hash ( ep.hostname
                                    , ep.port
                                    , ep.pid
                                    );
  }
}
