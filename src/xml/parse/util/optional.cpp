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

#include <xml/parse/util/optional.hpp>

namespace xml
{
  namespace parse
  {
    boost::optional<std::string> optional ( const xml_node_type* node
                                          , const Ch* attr
                                          )
    {
      if (node->first_attribute (attr))
      {
        return std::string ( node->first_attribute (attr)->value()
                           , node->first_attribute (attr)->value_size()
                           );
      }
      return boost::none;
    }
  }
}
