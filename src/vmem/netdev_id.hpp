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

#include <gpi-space/types.hpp>

#include <boost/any.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace fhg
{
  namespace vmem
  {
    //! Wrapper for gpi::netdev_id_t with to/from string support for
    //! command line option parsing.
    struct netdev_id
    {
      //! Let GASPI automatically choose netdev id.
      netdev_id();
      //! Depending on \a option either choose automatically ("auto"),
      //! device 0 ("0") or device 1 ("1").
      netdev_id (std::string const& option);
      //! Convert the current choice in the same way that the \c option
      //! constructor overload expects it.
      friend std::string to_string (netdev_id const&);
      //! Output \c to_string().
      friend std::ostream& operator<< (std::ostream&, netdev_id const&);
      //! Validate a command line option in the Boost.ProgramOptions
      //! framework.
      friend void validate
        (boost::any&, std::vector<std::string> const&, netdev_id*, int);
      //! Serialize using Boost.Serialization.
      template<typename Archive>
        void serialize (Archive&, unsigned int);

      //! The choice in a format that can be written to \c gaspi_config_t.
      gpi::netdev_id_t value;
    };
  }
}

#include <vmem/netdev_id.ipp>
