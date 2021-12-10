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

#include <boost/filesystem/path.hpp>

namespace iml
{
  namespace beegfs
  {
    //! A BeeGFS backed segment using a large file on a mountpoint
    //! that is assumed to be using local storage via BeeOND.
    struct IML_DLLEXPORT SegmentDescription
    {
      //! Create a description for a BeeGFS segment. The data and
      //! metadata will be stored in a directory at the given \a path.
      //! \note The mountpoint needs the \c tuneUseGlobalFileLocks
      //! option enabled.
      SegmentDescription (::boost::filesystem::path path);

      //! \note For serialization only.
      SegmentDescription() = default;

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      ::boost::filesystem::path path;
    };
  }
}

#include <iml/beegfs/SegmentDescription.ipp>
