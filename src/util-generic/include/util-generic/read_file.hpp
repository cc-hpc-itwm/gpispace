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

#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    template<typename T = std::string>
      T read_file (::boost::filesystem::path const& path)
    {
      std::ifstream ifs (path.string().c_str(), std::ifstream::binary);

      if (not ifs)
      {
        throw std::runtime_error
          ((::boost::format ("could not open %1%") % path).str());
      }

      ifs >> std::noskipws;

      return {std::istream_iterator<char> (ifs), std::istream_iterator<char>()};
    }

    template<typename T>
      T read_file_as (::boost::filesystem::path const& path)
    {
      return ::boost::lexical_cast<T> (read_file (path));
    }
  }
}
