// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
