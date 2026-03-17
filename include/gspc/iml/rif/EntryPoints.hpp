// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>
#include <gspc/rif/entry_point.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>


  namespace gspc::iml::rif
  {
    //! A collection of entry points, indexed by hostname.
    //! \note \c key and \c value.hostname *may* differ,
    //! the \c key being the hostname reported on the node,
    //! and \c value.hostname being the hostname used to
    //! connect the first time.
    using EntryPoints
      = std::unordered_map
          <std::string, gspc::rif::entry_point>;
  }
