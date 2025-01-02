// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace gpi
{
  using offset_t = std::uint64_t;
  using rank_t = unsigned short;
  using queue_desc_t = unsigned char;
  using size_t = std::uint64_t;
  using port_t = unsigned short;

  using notification_t = unsigned int;
  using notification_id_t = unsigned short;

  using timeout_t = unsigned long;
  using segment_id_t = unsigned char;
}
