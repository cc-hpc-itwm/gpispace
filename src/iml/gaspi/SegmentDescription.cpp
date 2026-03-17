// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/gaspi/SegmentDescription.hpp>


  namespace gspc::iml::gaspi
  {
    SegmentDescription::SegmentDescription
        ( std::size_t communication_buffer_size_
        , std::size_t communication_buffer_count_
        )
      : communication_buffer_size (communication_buffer_size_)
      , communication_buffer_count (communication_buffer_count_)
    {}
  }
