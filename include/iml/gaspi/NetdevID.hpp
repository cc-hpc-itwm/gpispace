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

#pragma once

#include <iml/detail/dllexport.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace boost
{
  class any;
}

namespace iml
{
  namespace gaspi
  {
    //! Wrapper for GPI's netdev_id_t with to/from string support for
    //! command line option parsing.
    struct IML_DLLEXPORT NetdevID
    {
      //! Depending on \a option either choose automatically ("auto"),
      //! device 0 ("0") or device 1 ("1").
      NetdevID (std::string const& option = "auto");

      //! Convert the current choice in the same way that the \c option
      //! constructor overload expects it.
      std::string to_string() const;
      //! Output \c to_string().
      IML_DLLEXPORT
        friend std::ostream& operator<< (std::ostream&, NetdevID const&);

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      //! The choice in a format that can be written to \c gaspi_config_t.
      int value;
    };

    //! Validate a command line option in the Boost.ProgramOptions
    //! framework.
    IML_DLLEXPORT
      void validate
        (::boost::any&, std::vector<std::string> const&, NetdevID*, int);
  }
}

#include <iml/gaspi/NetdevID.ipp>
