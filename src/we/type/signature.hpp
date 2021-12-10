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

#include <boost/variant.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      typedef typename ::boost::make_recursive_variant
                       < std::pair<std::string, std::string>
                       , std::pair<std::string, std::list<::boost::recursive_variant_>>
                       >::type field_type;

      typedef std::list<field_type> structure_type;

      typedef std::pair<std::string, structure_type> structured_type;

      typedef ::boost::variant<std::string, structured_type> signature_type;
    }
  }
}
