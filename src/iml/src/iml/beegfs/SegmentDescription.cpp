// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/beegfs/SegmentDescription.hpp>

#include <utility>

namespace iml
{
  namespace beegfs
  {
    SegmentDescription::SegmentDescription (::boost::filesystem::path path_)
      : path (std::move (path_))
    {}
  }
}
