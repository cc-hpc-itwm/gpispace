// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <string>
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

      entry_point (std::string const& hostname, unsigned short port, pid_t pid);

      //! parse input
      entry_point (std::string const& input);

      std::ostream& operator() (std::ostream&) const override;

      bool operator== (entry_point const&) const;

      //! \note Serialization only.
      entry_point();
      template<typename Archive> void serialize (Archive&, unsigned int);
    };
  }
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( fhg::rif::entry_point
  , ep
  , ep.hostname
  , ep.port
  , ep.pid
  )

#include <rif/entry_point.ipp>
