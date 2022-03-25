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

#include <we/expr/type/testing/types.hpp>

#include <we/type/signature/show.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>

namespace expr
{
  namespace type
  {
    namespace testing
    {
      std::ostream& operator<< (std::ostream& os, TypeDescription const& td)
      {
        return os
          << td.type
          << ", " << td.expression
          << ", " << pnet::type::signature::show (td.signature)
          ;
      }

      std::vector<TypeDescription> all_types()
      {
        return
          { {Control{}, "[]", "control"}
          , {Boolean{}, "true", "bool"}
          , {Int{}, "0", "int"}
          , {Long{}, "0l", "long"}
          , {UInt{}, "0u", "unsigned int"}
          , {ULong{}, "0ul", "unsigned long"}
          , {Float{}, "0f", "float"}
          , {Double{}, "0.0", "double"}
          , {Char{}, "'c'", "char"}
          , {String{}, "\"\"", "string"}
          , {Bitset{}, "{}", "bitset"}
          , {Bytearray{}, "y()", "bytearray"}
          , {List {Any()}, "List()", "list"}
          , {Set {Any()}, "Set{}", "set"}
          , {Map {Any(), Any()}, "Map[]", "map"}
          , {Struct {{}}, "Struct[]"
            , std::make_pair ( std::string ("struct")
                             , std::list<pnet::type::signature::field_type>{}
                             )
            }
          };
      }

      std::vector<TypeDescription> all_types_except (Type type)
      {
        auto all (all_types());
        decltype (all) filtered;

        std::copy_if ( std::begin (all), std::end (all)
                     , std::inserter (filtered, std::end (filtered))
                     , [&type] (auto const& te)
                       {
                         return te.type != type;
                       }
                     );

        return filtered;
      }
    }
  }
}
