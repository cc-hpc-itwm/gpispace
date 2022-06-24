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

#include <boost/variant.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      using field_type = typename ::boost::make_recursive_variant
        < std::pair<std::string, std::string>
        , std::pair<std::string, std::list< ::boost::recursive_variant_>>
        >::type;

      using structure_type = std::list<field_type>;

      using structured_type = std::pair<std::string, structure_type>;

      using signature_type = ::boost::variant<std::string, structured_type>;
    }
  }
}
