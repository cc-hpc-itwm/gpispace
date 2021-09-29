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

#include <we/type/value/name.hpp>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
#define NAME(_name,_value)                      \
      std::string const& _name()                \
      {                                         \
        static const std::string x (_value);    \
                                                \
        return x;                               \
      }

      NAME (CONTROL, "control")
      NAME (BOOL, "bool")
      NAME (INT, "int")
      NAME (LONG, "long")
      NAME (UINT, "unsigned int")
      NAME (ULONG, "unsigned long")
      NAME (FLOAT, "float")
      NAME (DOUBLE, "double")
      NAME (CHAR, "char")
      NAME (STRING, "string")
      NAME (BITSET,"bitset")
      NAME (BYTEARRAY, "bytearray")
      NAME (LIST, "list")
      NAME (SET, "set")
      NAME (MAP, "map")
      NAME (STRUCT, "struct")

#undef NAME

      namespace
      {
        std::list<std::string> init_type_names()
        {
          std::list<std::string> tn;

          tn.push_back (CONTROL());
          tn.push_back (BOOL());
          tn.push_back (INT());
          tn.push_back (LONG());
          tn.push_back (UINT());
          tn.push_back (ULONG());
          tn.push_back (FLOAT());
          tn.push_back (DOUBLE());
          tn.push_back (CHAR());
          tn.push_back (STRING());
          tn.push_back (BITSET());
          tn.push_back (BYTEARRAY());
          tn.push_back (LIST());
          tn.push_back (SET());
          tn.push_back (MAP());
          tn.push_back (STRUCT());

          return tn;
        }
      }

      std::list<std::string> type_names()
      {
        static std::list<std::string> tn (init_type_names());

        return tn;
      }
    }
  }
}
