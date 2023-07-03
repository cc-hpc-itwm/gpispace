// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/beegfs/SegmentDescription.hpp>
#include <iml/gaspi/SegmentDescription.hpp>

#include <boost/variant/variant.hpp>

namespace iml
{
  //! Parameters for how to allocate a specific segment, depending on
  //! segment type.
  //! \see gaspi::SegmentDescription, beegfs::SegmentDescription,
  //! Client::create_segment()
  using SegmentDescription
    = ::boost::variant < gaspi::SegmentDescription
                     , beegfs::SegmentDescription
                     >;
}
