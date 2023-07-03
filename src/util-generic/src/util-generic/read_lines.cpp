// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/read_lines.hpp>

#include <boost/format.hpp>

#include <cerrno>
#include <fstream>

namespace fhg
{
  namespace util
  {
    std::vector<std::string> read_lines (::boost::filesystem::path const& filename)
    {
      std::vector<std::string> lines;
      std::ifstream ifs (filename.string());
      if (!ifs)
      {
        throw std::runtime_error
          ( ( ::boost::format ("could not open file '%1%' for reading: %2%")
            % filename.string()
            % strerror (errno)
            ).str()
          );
      }
      for (std::string line; std::getline (ifs, line); )
      {
        lines.emplace_back (line);
      }
      return lines;
    }
  }
}
