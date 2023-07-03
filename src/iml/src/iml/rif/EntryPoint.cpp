// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
