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

#include <xml/parse/util/validprefix.hpp>

#include <xml/parse/error.hpp>

#include <xml/parse/rewrite/validprefix.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_prefix ( std::string const& name
                                , std::string const& type
                                , boost::filesystem::path const& path
                                )
    {
      if (rewrite::has_magic_prefix (name))
        {
          throw error::invalid_prefix (name, type, path);
        }

      return name;
    }
  }
}
