// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
