// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/gaspi/SegmentDescription.hpp>

namespace iml
{
  namespace gaspi
  {
    SegmentDescription::SegmentDescription
        ( std::size_t communication_buffer_size_
        , std::size_t communication_buffer_count_
        )
      : communication_buffer_size (communication_buffer_size_)
      , communication_buffer_count (communication_buffer_count_)
    {}
  }
}
