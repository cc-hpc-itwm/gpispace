// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>


  namespace gpi::pc
  {
    enum class is_global : bool
    {
      no,
      yes,
    };

    namespace type
    {
      using process_id_t = std::uint64_t;

      using flags_t = is_global;
      using name_t = std::string;
      using memcpy_id_t = std::uint64_t;
    }
  }
