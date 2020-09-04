// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <fstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    template<typename T>
      void write_file (boost::filesystem::path const& path, T const& x)
    {
      std::ofstream stream (path.string());

      if (!stream)
      {
        throw std::runtime_error
          ((boost::format ("Could not open %1% for writing.") % path).str());
      }

      stream << x;

      if (!stream)
      {
        throw std::runtime_error
          ((boost::format ("Could not write to %1%.") % path).str());
      }
    }
  }
}
