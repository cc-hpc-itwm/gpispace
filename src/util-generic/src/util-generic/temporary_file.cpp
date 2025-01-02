// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <FMT/boost/filesystem/path.hpp>
#include <fmt/core.h>
#include <util-generic/temporary_file.hpp>

namespace fhg::util::error::temporary_file
{
  auto already_exists
    ( ::boost::filesystem::path const& path
    ) -> std::logic_error
  {
    return std::logic_error
      { fmt::format ("Temporary file {} already exists.", path)
      };
  }
}
