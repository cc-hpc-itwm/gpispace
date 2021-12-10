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

#include <we/type/value.hpp>

#include <map>
#include <string>

namespace aggregate_sum
{
  class ValuesOnPorts
  {
  public:
     using Key = std::string;
     using Value = pnet::type::value::value_type;
     using Map = std::multimap<Key, Value>;

     ValuesOnPorts (Map map);

     Map const& map() const;

  protected:
     Map _values_on_ports;
  };
}
