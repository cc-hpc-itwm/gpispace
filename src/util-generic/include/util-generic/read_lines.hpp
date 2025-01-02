// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    [[deprecated ("use read_lines (std::filesystem::path")]]
    std::vector<std::string> read_lines (::boost::filesystem::path const&);
    std::vector<std::string> read_lines (std::filesystem::path const&);
  }
}
