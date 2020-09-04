// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#pragma once

#include <boost/serialization/split_free.hpp>

#include <util-generic/ostream/modifier.hpp>

#include <string>
#include <stdexcept>
#include <sstream>
#include <tuple>

#include <unistd.h>

namespace fhg
{
  namespace rif
  {
    struct entry_point : public fhg::util::ostream::modifier
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;

      //! \note Serialization only.
      entry_point() = default;

      entry_point (std::string const& hostname_, unsigned short port_, pid_t pid_)
        : hostname (hostname_)
        , port (port_)
        , pid (pid_)
      {}

      entry_point (std::string const& input)
      {
        std::istringstream iss (input);
        if (!(iss >> hostname >> port >> pid))
        {
          throw std::runtime_error
            ("parse error: expected 'host port pid': got '" + input + "'");
        }
      }
      virtual std::ostream& operator() (std::ostream& os) const override
      {
        return os << hostname << ' ' << port << ' ' << pid;
      }

      bool operator== (entry_point const& other) const
      {
        return std::tie (hostname, port, pid)
          == std::tie (other.hostname, other.port, other.pid);
      }
    };
  }
}

namespace std
{
  template<>
    struct hash<fhg::rif::entry_point>
  {
    std::size_t operator() (const fhg::rif::entry_point& ep) const
    {
      return std::hash<std::string>() (ep.string());
    }
  };
}

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , fhg::rif::entry_point& entry_point
                , const unsigned int
                )
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;
      ar & hostname;
      ar & port;
      ar & pid;
      entry_point = fhg::rif::entry_point (hostname, port, pid);
    }
    template<typename Archive>
      void save ( Archive& ar
                , fhg::rif::entry_point const& entry_point
                , const unsigned int
                )
    {
      ar & entry_point.hostname;
      ar & entry_point.port;
      ar & entry_point.pid;
    }

    template<typename Archive>
      void serialize ( Archive& ar
                     , fhg::rif::entry_point& entry_point
                     , const unsigned int version
                     )
    {
      boost::serialization::split_free (ar, entry_point, version);
    }
  }
}
