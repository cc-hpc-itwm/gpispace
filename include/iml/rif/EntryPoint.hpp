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

#include <cstddef>
#include <ostream>
#include <string>

namespace iml
{
  namespace rif
  {
    //! Information on how to contact a running RIF server, obtained
    //! from \c Rifds.
    struct IML_DLLEXPORT EntryPoint
    {
      std::string hostname;
      unsigned short port;
      pid_t pid;

      //! Create a RIF entry point from the string \a input.
      EntryPoint (std::string const& input);
      //! Print \a entry_point to \a os in a form that can be used to
      //! construct an \c EntryPoint from.
      IML_DLLEXPORT friend std::ostream& operator<<
        (std::ostream& os, EntryPoint const& entry_point);

      //! \note For serialization only.
      EntryPoint() = default;

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive& archive, unsigned int);

      //! Create a RIF entry point denoting the process \a pid_
      //! running on node \a hostname_, reachable at port \a port_.
      EntryPoint
        (std::string const& hostname_, unsigned short port_, pid_t pid_);

      friend bool operator== (EntryPoint const&, EntryPoint const&);
    };
  }
}

namespace std
{
  template<>
    struct IML_DLLEXPORT hash<iml::rif::EntryPoint>
  {
    std::size_t operator() (iml::rif::EntryPoint const&) const;
  };
}

#include <iml/rif/EntryPoint.ipp>
