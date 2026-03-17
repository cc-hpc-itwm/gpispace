// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/beegfs/SegmentDescription.hpp>
#include <gspc/iml/gaspi/SegmentDescription.hpp>

#include <variant>

#include <gspc/util/serialization/std/variant.hpp>

namespace gspc::iml
{
  //! Parameters for how to allocate a specific segment, depending on
  //! segment type.
  //! \see gaspi::SegmentDescription, beegfs::SegmentDescription,
  //! Client::create_segment()
  using SegmentDescription
    = std::variant < gaspi::SegmentDescription
                     , beegfs::SegmentDescription
                     >;
}
