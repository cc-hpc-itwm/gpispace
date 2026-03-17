// Copyright (C) 2014-2015,2020,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if GSPC_WITH_IML
#include <gspc/iml/SegmentDescription.hpp>
#endif

namespace gspc
{
  class vmem_allocation;

#if GSPC_WITH_IML
  namespace vmem
  {
    using beegfs_segment_description
      = gspc::iml::beegfs::SegmentDescription;
    using gaspi_segment_description
      = gspc::iml::gaspi::SegmentDescription;

    using segment_description
      = gspc::iml::SegmentDescription;
  }
#endif
}
