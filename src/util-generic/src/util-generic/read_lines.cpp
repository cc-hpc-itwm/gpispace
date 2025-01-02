// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/read_lines.hpp>

#include <cerrno>
#include <fmt/core.h>
#include <fmt/std.h>
#include <fstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    std::vector<std::string> read_lines (::boost::filesystem::path const& filename)
    {
      return read_lines (std::filesystem::path {filename.string()});
    }
    std::vector<std::string> read_lines (std::filesystem::path const& filename)
    {
      std::vector<std::string> lines;
      std::ifstream ifs (filename);
      if (!ifs)
      {
        throw std::runtime_error
          { fmt::format ( "could not open file '{}' for reading: {}"
                        , filename
                        , strerror (errno)
                        )
          };
      }
      for (std::string line; std::getline (ifs, line); )
      {
        lines.emplace_back (line);
      }
      return lines;
    }
  }
}
