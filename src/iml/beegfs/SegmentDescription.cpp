// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/beegfs/SegmentDescription.hpp>

#include <utility>


  namespace gspc::iml::beegfs
  {
    SegmentDescription::SegmentDescription (std::filesystem::path path_)
      : path (std::move (path_))
    {}
  }
