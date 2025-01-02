// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <FMT/boost/filesystem/path.hpp>
#include <fmt/core.h>
#include <util-generic/temporary_path.hpp>
#include <utility>

namespace fhg::util::error
{
  path_already_exists::path_already_exists (::boost::filesystem::path p)
    : std::logic_error {fmt::format ("Temporary path {} already exists.", p)}
    , path {std::move (p)}
  {}
}
