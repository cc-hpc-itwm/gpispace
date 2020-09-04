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

#include <we/type/signature/is_literal.hpp>
#include <we/type/value/name.hpp>

#include <set>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        std::set<std::string> init_literal_names()
        {
          std::set<std::string> ln;

          ln.insert (value::CONTROL());
          ln.insert (value::BOOL());
          ln.insert (value::INT());
          ln.insert (value::LONG());
          ln.insert (value::UINT());
          ln.insert (value::ULONG());
          ln.insert (value::FLOAT());
          ln.insert (value::DOUBLE());
          ln.insert (value::CHAR());
          ln.insert (value::STRING());
          ln.insert (value::BITSET());
          ln.insert (value::BYTEARRAY());
          ln.insert (value::LIST());
          ln.insert (value::SET());
          ln.insert (value::MAP());

          ln.insert ("stack");

          return ln;
        }
      }

      bool is_literal (const std::string& tname)
      {
        static std::set<std::string> ln (init_literal_names());

        return ln.find (tname) != ln.end();
      }
    }
  }
}
