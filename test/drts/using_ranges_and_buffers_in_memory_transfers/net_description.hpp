// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include <string>


  namespace drts::test
  {
    std::string net_description
      (std::string const&, std::optional<bool> allow_empty_ranges, bool);
  }
