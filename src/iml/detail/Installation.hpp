// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/dllexport.hpp>

#include <boost/filesystem/path.hpp>

namespace iml
{
  namespace detail
  {
    class FHG_UTIL_DLLEXPORT Installation
    {
    public:
      Installation();

      ::boost::filesystem::path const server_binary;
      ::boost::filesystem::path const rifd_binary;
    };
  }
}
