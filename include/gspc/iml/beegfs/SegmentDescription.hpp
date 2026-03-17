// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>


  namespace gspc::iml::beegfs
  {
    //! A BeeGFS backed segment using a large file on a mountpoint
    //! that is assumed to be using local storage via BeeOND.
    struct GSPC_EXPORT SegmentDescription
    {
      //! Create a description for a BeeGFS segment. The data and
      //! metadata will be stored in a directory at the given \a path.
      //! \note The mountpoint needs the \c tuneUseGlobalFileLocks
      //! option enabled.
      SegmentDescription (std::filesystem::path path);

      //! \note For serialization only.
      SegmentDescription() = default;

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      std::filesystem::path path;
    };
  }


#include <gspc/iml/beegfs/SegmentDescription.ipp>
