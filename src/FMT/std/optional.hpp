// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fmt/core.h>
#include <fmt/std.h>
#include <iostream>

namespace std
{
  template<typename T>
    auto operator<<
      ( std::ostream& os
      , std::optional<T> const& o
      ) -> std::ostream&
  {
    fmt::print (os, "{}", o);

    return os;
  }
}
