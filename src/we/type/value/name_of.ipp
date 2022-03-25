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

#include <we/type/value/name_of.hpp>
#include <we/type/value/name.hpp>

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
#define NAME_OF(_name, _type...)                                         \
      template<> inline std::string const& name_of<_type> (_type const&) \
      {                                                                  \
        return _name();                                                  \
      }

      NAME_OF (CONTROL, we::type::literal::control)
      NAME_OF (BOOL, bool)
      NAME_OF (INT, int)
      NAME_OF (LONG, long)
      NAME_OF (UINT, unsigned int)
      NAME_OF (ULONG, unsigned long)
      NAME_OF (FLOAT, float)
      NAME_OF (DOUBLE, double)
      NAME_OF (CHAR, char)
      NAME_OF (STRING, std::string)
      NAME_OF (BITSET, bitsetofint::type)
      NAME_OF (BYTEARRAY, we::type::bytearray)
      NAME_OF (LIST, std::list<value_type>)
      NAME_OF (SET, std::set<value_type>)
      NAME_OF (MAP, std::map<value_type,value_type>)
      NAME_OF (STRUCT, structured_type)

#undef NAME_OF
    }
  }
}
