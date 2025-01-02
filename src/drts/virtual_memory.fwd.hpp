// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/SegmentDescription.hpp>

namespace gspc
{
  class vmem_allocation;

  namespace vmem
  {
    using beegfs_segment_description = iml::beegfs::SegmentDescription;
    using gaspi_segment_description = iml::gaspi::SegmentDescription;

    using segment_description = iml::SegmentDescription;
  }
}
