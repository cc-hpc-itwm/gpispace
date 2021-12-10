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

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      expr::parse::parser generic_we_parse ( std::string const& input
                                           , std::string const& descr
                                           );
      expr::parse::parser we_parse ( std::string const& input
                                   , std::string const& descr
                                   , std::string const& type
                                   , std::string const& name
                                   , ::boost::filesystem::path const&
                                   );
   }
  }
}
