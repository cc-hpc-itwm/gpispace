// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fmt/core.h>
#include <fmt/std.h>
#include <util-generic/write_file.hpp>

namespace fhg::util::error::write_file
{
  auto could_not_open
    ( std::filesystem::path const& path
    ) -> std::runtime_error
  {
    return std::runtime_error
      { fmt::format ("Could not open {} for writing.", path)
      };
  }

  auto could_not_write
    ( std::filesystem::path const& path
    ) -> std::runtime_error
  {
    return std::runtime_error
      { fmt::format ("Could not write to {}.", path)
      };
  }
}
