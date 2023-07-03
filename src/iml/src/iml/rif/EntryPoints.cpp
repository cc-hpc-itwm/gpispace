// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/rif/EntryPoints.hpp>

#include <util-generic/read_lines.hpp>

#include <fstream>
#include <stdexcept>

namespace iml
{
  namespace rif
  {
    EntryPoints read_from_file (::boost::filesystem::path const& path)
    try
    {
      EntryPoints entry_points;

      for (auto const& line : fhg::util::read_lines (path))
      {
        auto const pos (line.find_first_of (' '));

        if (pos == std::string::npos)
        {
          throw std::logic_error
            ( "Failed to parse: expected 'real-host host port pid', got '"
              + line + "'"
            );
        }

        entry_points.emplace ( line.substr (0, pos)
                             , line.substr (pos + 1, std::string::npos)
                             );
      }

      return entry_points;
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
            ("unable to read entry points from file '" + path.string() + "'")
        );
    }

    void write_to_file ( EntryPoints const& entry_points
                       , ::boost::filesystem::path const& path
                       )
    try
    {
      std::ofstream file (path.string());
      if (!file)
      {
        throw std::runtime_error ("unable to open file");
      }

      // Duplicated in iml-bootstrap-rifd, but additionally including
      // the real hostname. Keep in sync when changing!
      for (auto const& entry_point : entry_points)
      {
        if (!(file << entry_point.first << ' ' << entry_point.second << '\n'))
        {
          throw std::runtime_error ("unable to write entry point");
        }
      }
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
            ("unable to write entry points to file '" + path.string() + "'")
        );
    }
  }
}
