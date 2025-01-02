// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fmt/core.h>
#include <fmt/std.h>
#include <util-generic/read_file.hpp>

namespace fhg::util::error::read_file
{
  auto could_not_open
    ( std::filesystem::path const& path
    ) -> std::runtime_error
  {
    return std::runtime_error {fmt::format ("could not open {}", path)};
  }
}
