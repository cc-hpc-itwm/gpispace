// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>
#include <iml/rif/EntryPoint.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>

namespace iml
{
  namespace rif
  {
    //! A collection of entry points, indexed by hostname.
    //! \note \c key and \c value.hostname *may* differ, the \c key
    //! being the hostname reported on the node, and \c value.hostname
    //! being the hostname used to connect the first time.
    using EntryPoints = std::unordered_map<std::string, EntryPoint>;

    //! Write all given \a entry_points to a file at \a path.
    //! \note The file is overwritten, not appended.
    //! \note Compatible with the input expected for the
    //! iml-teardown-rifd binary.
    IML_DLLEXPORT void write_to_file
      (EntryPoints const& entry_points, ::boost::filesystem::path const& path);

    //! Read a collection of entry points from the file at \a path.
    //! \note Compatible with the output of the iml-bootstrap-rifd
    //! binary.
    IML_DLLEXPORT EntryPoints read_from_file
      (::boost::filesystem::path const& path);
  }
}
